#!/usr/bin/env python3
"""Set the localized application descriptions without changing the SMDH icons."""
from pathlib import Path
import sys

# Language indices follow libctru's CFG_Language / the SMDH title table.
DESCRIPTIONS = {
    1: "Launcher and updater for Dan Franza's 3DS port",
    2: 'Lanceur et mises à jour du port 3DS de Dan Franza',
    4: 'Launcher e aggiornamenti del port 3DS di Dan Franza',
    5: 'Lanzador y actualizador del port 3DS de Dan Franza',
    9: 'Inicializador e atualizador do port 3DS de Dan Franza',
}


def field(text, size):
    encoded = text.encode('utf-16le')
    if len(encoded) > size - 2:
        raise ValueError('Application description exceeds SMDH field capacity')
    return encoded + bytes(size - len(encoded))


def localize(data):
    if len(data) != 0x36C0 or data[:4] != b'SMDH':
        raise ValueError('Invalid SMDH file')
    output = bytearray(data)
    for index in range(16):
        at = 8 + index * 0x200
        output[at:at + 0x80] = field('PokéSurvivor 3DS - Dan Franza', 0x80)
        output[at + 0x80:at + 0x180] = field(DESCRIPTIONS.get(index, DESCRIPTIONS[1]), 0x100)
        output[at + 0x180:at + 0x200] = field('Dan Franza', 0x80)
    return output


if __name__ == '__main__':
    path = Path(sys.argv[1])
    path.write_bytes(localize(path.read_bytes()))
