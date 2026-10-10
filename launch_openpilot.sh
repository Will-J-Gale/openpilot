#!/usr/bin/env bash

cd /data/openpilot
./ajarpilot/selector/selector
program=$(cat /data/.launch_program)

if [[ $program == "openpilot" ]]; then
  ./launch_chffrplus.sh
else
  ./launch_ajarpilot.sh
fi