import time
from subprocess import Popen

PROCESSES = [
    ("openpilot/system/camerad", "./camerad"),
    ("ajarpilot/ui", "./ui"),
]

def main():
    processes = []

    for cwd, command in PROCESSES:
        p = Popen([command], cwd=cwd)
        processes.append((command, p))

    while(True):
        try:
            time.sleep(0.5)
        except KeyboardInterrupt:
            break

    for command, p in processes:
        print(f"Terminating {command}")
        p.terminate()
        p.wait(10)

if __name__ == "__main__":
    main()