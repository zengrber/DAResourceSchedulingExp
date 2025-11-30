import glob
import re

import pandas as pd
import matplotlib.pyplot as plt

all_dfs = []

for path in glob.glob("results/results_*.csv"):
    m = re.search(r"results_([0-9.]+)\.csv", path)
    if not m:
        print(f"[WARN] Cannot extract misreportProb from filename: {path}")
        continue

    p_str = m.group(1)
    try:
        p_val = float(p_str)
    except ValueError:
        print(f"[WARN] Cannot parse float from '{p_str}' in {path}")
        continue

    df = pd.read_csv(path)
    df["misreportProb"] = p_val
    all_dfs.append(df)

if not all_dfs:
    raise RuntimeError("No valid results_*.csv files found.")

df = pd.concat(all_dfs, ignore_index=True)

numeric_cols = ["totalJobs", "finishedJobs", "avgCompletion", "avgWaiting", "util"]
for col in numeric_cols:
    df[col] = pd.to_numeric(df[col], errors="coerce")

grouped = (
    df.groupby(["misreportProb", "type"])[numeric_cols]
      .mean()
      .reset_index()
)

grouped = grouped.sort_values("misreportProb")

types_info = [
    ("base_truth", "Base (Truthful)"),
    ("base_strat", "Base (Strategic)"),
    ("da_truth",   "DA (Truthful)"),
    ("da_strat",   "DA (Strategic)"),
]

def plot_metric(metric_name, ylabel):
    plt.figure(figsize=(8, 5))

    for t, label in types_info:
        subset = grouped[grouped["type"] == t]
        if subset.empty:
            continue

        plt.plot(
            subset["misreportProb"],
            subset[metric_name],
            marker="o",
            label=label,
        )

    plt.xlabel("misreportProb (p)")
    plt.ylabel(ylabel)
    plt.title(f"{ylabel} vs misreportProb")
    plt.grid(True, linestyle="--", alpha=0.4)
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"graphs/{metric_name}_vs_p.png", dpi=150)
    plt.show()

plot_metric("finishedJobs",  "Finished Jobs")
plot_metric("avgCompletion", "Average Completion Time")
plot_metric("avgWaiting",    "Average Waiting Time")
