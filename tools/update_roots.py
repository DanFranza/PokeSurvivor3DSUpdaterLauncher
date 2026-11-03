#!/usr/bin/env python3
"""Convert a trusted PEM root bundle to the launcher's length-prefixed DER file."""
import argparse
from pathlib import Path
import re
import ssl
import struct

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('pem', type=Path)
parser.add_argument('--output', type=Path, default=Path(__file__).parents[1] / 'romfs/roots.bin')
args = parser.parse_args()
blocks = re.findall(r'-----BEGIN CERTIFICATE-----.*?-----END CERTIFICATE-----', args.pem.read_text(), re.S)
if not blocks:
    parser.error('Aucun certificat PEM trouvé.')
output = bytearray()
for block in blocks:
    der = ssl.PEM_cert_to_DER_cert(block)
    if not 1 <= len(der) <= 8192:
        parser.error('Certificat trop volumineux.')
    output += struct.pack('<I', len(der)) + der
args.output.write_bytes(output)
print(f'{len(blocks)} certificats écrits dans {args.output}')
