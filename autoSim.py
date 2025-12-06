import subprocess
import pathlib
import re

# -----------------------------
# 可调参数
# -----------------------------

# misreport 概率
MISREPORT_PROBS = [0.01, 0.1, 0.25, 0.5, 0.75, 0.99]

# misreport 幅度（1%, 10%, 50%, 100%, 200%, 1000%）
MISREPORT_ALPHAS = [0.01, 0.1, 0.5, 1.0, 2.0, 10.0]

# 文件路径（按你项目真实情况调整）
BASE_CONFIG_PATH = pathlib.Path("config_base.txt")
WORK_CONFIG_PATH = pathlib.Path("config.txt")
SIM_EXE = pathlib.Path("build\\sim.exe")  # Windows 下也可以用 "build\\sim.exe"


# -----------------------------
# 工具：替换 config 里的某一行“key = value”
# -----------------------------
def replace_param(text: str, key: str, value) -> str:
    """
    把形如 'key = xxx' 的行替换为 'key = value'。
    要求这一行在 config_base.txt 里存在。
    """
    pattern = rf"^{key}\s*=\s*.*$"
    replacement = f"{key} = {value}"
    new_text, count = re.subn(pattern, replacement, text, flags=re.MULTILINE)
    if count == 0:
        raise ValueError(f"param '{key}' not found! Check config_base.txt")
    return new_text


def main():
    if not BASE_CONFIG_PATH.exists():
        raise FileNotFoundError(f"Base config {BASE_CONFIG_PATH} not exist")

    if not SIM_EXE.exists():
        raise FileNotFoundError(f"Executable binary {SIM_EXE} not found, please build first")

    base_cfg = BASE_CONFIG_PATH.read_text(encoding="utf-8")

    for p in MISREPORT_PROBS:
        for alpha in MISREPORT_ALPHAS:
            # 生成当前组合对应的 config 内容
            cfg_text = base_cfg
            cfg_text = replace_param(cfg_text, "misreportProb", p)
            cfg_text = replace_param(cfg_text, "misreportAlpha", alpha)

            # 写到 config.txt
            WORK_CONFIG_PATH.write_text(cfg_text, encoding="utf-8")

            print(f"===> Running sim.exe with p={p}, alpha={alpha} ...")
            # 调用模拟器
            result = subprocess.run(
                [str(SIM_EXE), str(WORK_CONFIG_PATH)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )

            # 简单输出一下日志（可按需要调整）
            if result.returncode != 0:
                print(f"[ERROR] sim.exe failed for p={p}, alpha={alpha}")
                print("stdout:")
                print(result.stdout)
                print("stderr:")
                print(result.stderr)
                # 这里可以选择 continue 或直接 raise
                raise RuntimeError("sim.exe returned non-zero exit code")
            else:
                print(f"[OK] p={p}, alpha={alpha} done.")
                # 如果你需要，可以把 stdout 写到单独 log 文件
                # pathlib.Path(f"log_{p}_{alpha}.txt").write_text(result.stdout, encoding="utf-8")


if __name__ == "__main__":
    main()
