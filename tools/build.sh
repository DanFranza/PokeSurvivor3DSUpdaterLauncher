#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"
export DEVKITARM="${DEVKITARM:-$DEVKITPRO/devkitARM}"
export PATH="$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH"
make -j4
if [ "${1:-}" = "--cia" ]; then
    "${MAKEROM:-makerom}" -f cia -o pokesurvivor-launcher.cia \
        -elf pokesurvivor-launcher.elf -rsf launcher.rsf \
        -icon pokesurvivor-launcher.smdh -banner assets/banner.bnr -target t
fi
