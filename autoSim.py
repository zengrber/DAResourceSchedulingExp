import subprocess
import pathlib
import re

# misreport probability
MISREPORT_PROBS = [0.01, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.99]

# misreport alpha
MISREPORT_ALPHAS = [0.01, 0.1, 0.5, 1.0, 2.0, 10.0]

BASE_CONFIG_PATH = pathlib.Path("config_base.txt")
WORK_CONFIG_PATH = pathlib.Path("config.txt")
SIM_EXE = pathlib.Path("build\\sim.exe")  


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
            # generate config text
            cfg_text = base_cfg
            cfg_text = replace_param(cfg_text, "misreportProb", p)
            cfg_text = replace_param(cfg_text, "misreportAlpha", alpha)

            # write to config
            WORK_CONFIG_PATH.write_text(cfg_text, encoding="utf-8")

            print(f"===> Running sim.exe with p={p}, alpha={alpha} ...")
            # run simulator
            result = subprocess.run(
                [str(SIM_EXE), str(WORK_CONFIG_PATH)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )

            if result.returncode != 0:
                print(f"[ERROR] sim.exe failed for p={p}, alpha={alpha}")
                print("stdout:")
                print(result.stdout)
                print("stderr:")
                print(result.stderr)
                raise RuntimeError("sim.exe returned non-zero exit code")
            else:
                print(f"[OK] p={p}, alpha={alpha} done.")


if __name__ == "__main__":
    main()
