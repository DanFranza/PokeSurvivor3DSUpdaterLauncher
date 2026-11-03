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
parser.add_argument('--github', action='store_true', help='Use only the root authorities used by GitHub and its download hosts.')
args = parser.parse_args()
blocks = re.findall(r'-----BEGIN CERTIFICATE-----.*?-----END CERTIFICATE-----', args.pem.read_text(), re.S)
if not blocks:
    parser.error('Aucun certificat PEM trouvé.')
# A full desktop trust store can exhaust the console HTTP service's resources.
# These are trust anchors, never server/leaf certificates.
names = {'Sectigo Public Server Authentication Root E46', 'Sectigo Public Server Authentication Root R46',
         'ISRG Root X1', 'ISRG Root X2', 'USERTrust RSA Certification Authority',
         'USERTrust ECC Certification Authority', 'DigiCert Global Root G2', 'DigiCert Global Root G3'}
if args.github:
    import tempfile
    selected = []
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / 'certificate.pem'
        for block in blocks:
            path.write_text(block)
            details = ssl._ssl._test_decode_cert(str(path))
            common_names = {value for group in details['subject'] for key, value in group if key == 'commonName'}
            if common_names & names:
                selected.append(block)
                names -= common_names
    if names:
        parser.error('Missing GitHub trust anchors: ' + ', '.join(sorted(names)))
    blocks = selected
output = bytearray()
for block in blocks:
    der = ssl.PEM_cert_to_DER_cert(block)
    if not 1 <= len(der) <= 8192:
        parser.error('Certificat trop volumineux.')
    output += struct.pack('<I', len(der)) + der
args.output.write_bytes(output)
args.output.with_suffix('.pem').write_text(''.join(block + '\n' for block in blocks))
print(f'{len(blocks)} certificats écrits dans {args.output}')
