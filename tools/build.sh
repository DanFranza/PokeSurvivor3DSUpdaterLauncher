#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"
export DEVKITARM="${DEVKITARM:-$DEVKITPRO/devkitARM}"
export PATH="$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH"
python3 tools/generate_translations.py --check
make -j4
if [ "${1:-}" = "--cia" ]; then
    launcher_title_version=$(python3 -c 'import re; from pathlib import Path; a,b,c=map(int,re.search(r"LAUNCHER_VERSION \"(\d+)\.(\d+)\.(\d+)\"",Path("source/version.h").read_text()).groups()); assert a<64 and b<64 and c<16; print((a<<10)|(b<<4)|c)')
    "${MAKEROM:-makerom}" -f cia -o pokesurvivor-launcher.cia \
        -elf pokesurvivor-launcher.elf -rsf launcher.rsf \
        -icon pokesurvivor-launcher.smdh -banner assets/banner.bnr -target t -ver "$launcher_title_version"
fi
