#!/usr/bin/env python3

import os
import time
from subprocess import Popen

from openpilot.system.hardware import TICI

LOG_ROOT = "/data/custom_drives" if TICI else os.path.expanduser("~/.config/ajarpilot")
os.environ["LOG_ROOT"] = LOG_ROOT
os.makedirs(LOG_ROOT, exist_ok=True)

PROCESSES = [
    ("ajarpilot/tools/camerad", "./camerad_sim"),
    ("ajarpilot/ui", "./ui"),
    ("ajarpilot/modeld", "./modeld"),
]

GREEN = "\033[32m"
RED = "\033[31m"
RESET = "\033[0m"

def main():
    processes = []

    for cwd, command in PROCESSES:
        p = Popen([command], cwd=cwd)
        processes.append((command, p))

    while(True):
        try:
            message = "Processes: "
            for command, p in processes:
                alive = p.poll() is None
                colour = GREEN if alive else RED
                command_text = command.replace("./", "")
                message += f"{colour}{command_text}{RESET} "

            print(message)

            time.sleep(0.5)
        except KeyboardInterrupt:
            break

    for command, p in processes:
        print(f"Terminating {command}")
        p.terminate()
        p.wait(10)

if __name__ == "__main__":
    main()