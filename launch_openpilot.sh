#!/usr/bin/env bash

# exec ./launch_chffrplus.sh
cd /data/openpilot
./ajarpilot/selector/selector
program=$(cat /data/.launch_program)
echo $program

if [[ $program == "openpilot" ]]; then
  ./launch_chffrplus.sh
else
  ./launch_ajarpilot.sh
fi