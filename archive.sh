#!/usr/bin/env bash
set -euxo pipefail

for strProject in "kitelite" "SensorCal" "libcalib"; do
	repomix --remote https://github.com/bruceoberg/${strProject} --output ~/Downloads/${strProject}-latest.xml
done
