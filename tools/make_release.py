#!/usr/bin/env python3
"""Prepare the version manifest; never upload or create a release."""
import argparse
import hashlib
from pathlib import Path
import re
from urllib.parse import urlsplit


def manifest(binary: Path, version: str, url: str) -> str:
    if not re.fullmatch(r'[A-Za-z0-9._-]{1,48}', version):
        raise ValueError('Version: utiliser 1 à 48 lettres, chiffres, points, tirets ou underscores.')
    parsed = urlsplit(url)
    if (parsed.scheme != 'https' or not parsed.hostname or parsed.username or
            parsed.password or parsed.port or parsed.fragment or len(url) > 2000 or
            any(ord(c) <= 32 or ord(c) >= 127 for c in url)):
        raise ValueError('Une URL HTTPS publique sans identifiants est requise.')
    size = binary.stat().st_size
    if not 32 <= size <= 512 * 1024 * 1024:
        raise ValueError('Taille 3DSX hors limites.')
    digest = hashlib.sha256()
    with binary.open('rb') as stream:
        if stream.read(4) != b'3DSX':
            raise ValueError('Le fichier ne possède pas la signature 3DSX.')
        stream.seek(0)
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return f'format=1\nversion={version}\nurl={url}\nsize={size}\nsha256={digest.hexdigest()}\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', type=Path)
    parser.add_argument('--version', required=True)
    parser.add_argument('--url', required=True, help='URL GitHub de cet asset, avec tag fixe')
    parser.add_argument('--output', type=Path, default=Path('pokesurvivor-update.ini'))
    args = parser.parse_args()
    try:
        text = manifest(args.binary, args.version, args.url)
    except (ValueError, OSError) as exc:
        parser.error(str(exc))
    args.output.write_text(text, encoding='ascii')
    print(f'Manifeste créé : {args.output}')


if __name__ == '__main__':
    main()
