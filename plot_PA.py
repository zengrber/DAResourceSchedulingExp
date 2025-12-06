import pandas as pd
import matplotlib.pyplot as plt
import glob
import os
import re

def is_float(s):
    pattern = re.compile(r'^[-+]?[0-9]*\.?[0-9]+$')
    return bool(pattern.match(s))

# -----------------------------
# 配置区
# -----------------------------

# 你想看的指标：在这里改成 "finishedJobs" / "avgWaiting" / "avgCompletion" / "util"
METRIC = "finishedJobs"

# 4 种策略，对应 CSV 里的 type 字段
STRATEGIES = ["base_truth", "base_strat", "da_truth", "da_strat"]

# results 文件的通配符（按需要修改路径）
PATTERN = "results\\results_*.csv"

# -----------------------------
# 读取所有 CSV，并从文件名解析 p 和 alpha
# -----------------------------

dfs = []

for path in glob.glob(PATTERN):
    filename = os.path.basename(path)  # e.g. "results_0.0_0.01.csv"
    core = filename[len("results_"):-4]  # 去掉 "results_" 和 ".csv" -> "0.0_0.01"
    try:
        p_str, alpha_str = core.split("_")
    except ValueError:
        print(f"Invalid filename {filename}")
        continue
    
    if is_float(p_str):
        p = float(p_str)
    else:
        continue
    if is_float(alpha_str):
        alpha = float(alpha_str)
    else: 
        continue

    df = pd.read_csv(path)
    df["misreportProb"] = p
    df["misreportAlpha"] = alpha
    dfs.append(df)

if not dfs:
    raise RuntimeError("no results_*.csv is matched")

all_data = pd.concat(dfs, ignore_index=True)

# 简单检查一次
print("detected alpha:", sorted(all_data["misreportAlpha"].unique()))
print("detected misreportProb :", sorted(all_data["misreportProb"].unique()))
print("deteced strates:", all_data["type"].unique())

# -----------------------------
# 为每种策略画一张图
# -----------------------------

for strategy in STRATEGIES:
    data_s = all_data[all_data["type"] == strategy]
    if data_s.empty:
        print(f"strategy type = {strategy} not found")
        continue

    plt.figure(figsize=(8, 5))

    # 对每个 alpha 画一条折线
    for alpha in sorted(data_s["misreportAlpha"].unique()):
        sub = data_s[data_s["misreportAlpha"] == alpha]

        # 按 misreportProb 分组，对多个 seed 取平均
        grouped = sub.groupby("misreportProb")[METRIC].mean().sort_index()

        x = grouped.index.values        # misreportProb
        y = grouped.values              # 指标均值

        # 不指定颜色，让 matplotlib 自己选
        plt.plot(x, y, marker="o", label=f"α = {alpha}")

    plt.title(f"{strategy}: {METRIC} vs misreport probability")
    plt.xlabel("misreport probability p")
    plt.ylabel(METRIC)
    plt.grid(True, linestyle="--", alpha=0.3)
    plt.legend()
    plt.tight_layout()

    plt.savefig(f"graphs\\{strategy}_{METRIC}.png", dpi=300)

plt.show()
