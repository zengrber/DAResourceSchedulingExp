import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results/results_0.250000.csv")

numeric_cols = ["totalJobs", "finishedJobs", "avgCompletion", "avgWaiting", "util"]
for col in numeric_cols:
    df[col] = pd.to_numeric(df[col], errors="coerce")

types_info = [
    ("base_truth", "Base (Truthful)"),
    ("base_strat", "Base (Strategic)"),
    ("da_truth",   "DA (Truthful)"),
    ("da_strat",   "DA (Strategic)"),
]

def plot_metric(metric_name, ylabel):
    plt.figure(figsize=(8, 5))

    for t, label in types_info:
        subset = df[df["type"] == t].copy()
        if subset.empty:
            continue

        if "seed" in subset.columns:
            subset = subset.sort_values("seed")

        subset = subset.reset_index(drop=True)
        subset["run_idx"] = subset.index  # 0,1,2,...

        plt.plot(
            subset["run_idx"],
            subset[metric_name],
            marker="o",
            label=label,
        )

    plt.xlabel("Run index (per type)")
    plt.ylabel(ylabel)
    plt.title(f"{ylabel} across runs")
    plt.grid(True, linestyle="--", alpha=0.4)
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"graphs/{metric_name}_runs.png", dpi=150)
    plt.show()

plot_metric("finishedJobs",  "Finished Jobs")
plot_metric("avgCompletion", "Average Completion Time")
plot_metric("avgWaiting",    "Average Waiting Time")
