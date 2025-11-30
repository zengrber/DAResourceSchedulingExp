import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np

def plot_schedule(csv_path, title, out_png=None):
    df = pd.read_csv(csv_path)

    servers = sorted(df["serverId"].unique())
    job_ids = sorted(df["jobId"].unique()) or [0]
    max_colors = 20
    cmap = plt.get_cmap("tab20", max_colors)

    fig, ax = plt.subplots(figsize=(14, 6))

    y_ticks = []
    y_labels = []

    # 高度布局参数
    lane_height = 6
    lane_gap    = 2
    server_gap  = 6

    current_y = 0

    for sid in servers:
        sub = df[df["serverId"] == sid].copy()
        sub.sort_values("startTime", inplace=True)

        # ----- 为该 server 做 lane 排布 -----
        lanes_end = []      # 每个 lane 当前结束时间
        plotted_jobs = []   # (lane_id, start, end, jobId)

        for _, row in sub.iterrows():
            start = row["startTime"]
            end   = row["endTime"]
            jid   = int(row["jobId"])

            lane_id = None
            for i, end_time in enumerate(lanes_end):
                if start >= end_time:
                    lane_id = i
                    lanes_end[i] = end
                    break
            if lane_id is None:
                lane_id = len(lanes_end)
                lanes_end.append(end)

            plotted_jobs.append((lane_id, start, end, jid))

        num_lanes = max(1, len(lanes_end))
        server_height = num_lanes * (lane_height + lane_gap) - lane_gap

        # y 轴标签放在该 server 中间
        y_ticks.append(current_y + server_height / 2)
        y_labels.append(f"Server {sid}")

        # ----- 画 job 条 -----
        for lane_id, start, end, jid in plotted_jobs:
            lane_y = current_y + lane_id * (lane_height + lane_gap)
            dur = end - start
            color = cmap(jid % max_colors)

            ax.broken_barh(
                [(start, dur)],
                (lane_y, lane_height),
                facecolors=color,
                edgecolors="black",
                linewidth=0.5,
            )
            ax.text(
                start + dur / 2,
                lane_y + lane_height / 2,
                str(jid),
                ha="center",
                va="center",
                fontsize=6,
                color="white",
            )

        current_y += server_height + server_gap

    ax.set_xlabel("Time")
    ax.set_yticks(y_ticks)
    ax.set_yticklabels(y_labels)
    ax.set_title(title)
    ax.grid(True, axis="x", linestyle="--", alpha=0.4)

    # =============================
    #  叠加 overall utilization 折线
    # =============================
    # 1) 决定时间轴取样点（整数时间刻度）
    t_min = df["startTime"].min()
    t_max = df["endTime"].max()
    times = np.arange(t_min, t_max + 1)

    # 2) 计算每个时刻的总使用量
    used = []
    for t in times:
        # 当前时刻正在跑的 job： start <= t < end
        running = df[(df["startTime"] <= t) & (df["endTime"] > t)]
        used.append(running["demand"].sum())

    used = np.array(used, dtype=float)

    # 3) 计算总 capacity（如果有 serverCap 就用；否则用 used 的最大值当 scale）
    if "serverCap" in df.columns:
        caps = df.groupby("serverId")["serverCap"].first()
        total_cap = caps.sum()
    else:
        total_cap = used.max() if used.max() > 0 else 1.0

    util = used / total_cap  # 0~1

    ax2 = ax.twinx()
    ax2.plot(times, util, color="black", linewidth=1.5, label="Utilization")
    ax2.set_ylabel("Total utilization")
    ax2.set_ylim(0, 1.05)

    # 合并两个轴的 legend
    handles1, labels1 = ax.get_legend_handles_labels()
    handles2, labels2 = ax2.get_legend_handles_labels()
    if handles2:
        ax2.legend(handles2, labels2, loc="upper right")

    plt.tight_layout()
    if out_png:
        plt.savefig(out_png, dpi=150)
    plt.show()

if __name__ == "__main__":
    plot_schedule("results/schedule_base_truth.csv",  "Base Truthful",  "graphs/schedule_base_truth.png")
    plot_schedule("results/schedule_base_strat.csv",  "Base Strategic", "graphs/schedule_base_strat.png")
    plot_schedule("results/schedule_da_truth.csv",    "DA Truthful",    "graphs/schedule_da_truth.png")
    plot_schedule("results/schedule_da_strat.csv",    "DA Strategic",   "graphs/schedule_da_strat.png")
