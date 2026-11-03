import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('release', Path(__file__).parents[1] / 'tools/make_release.py')
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


class ReleaseTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.binary = Path(self.directory.name) / 'game.3dsx'
        self.binary.write_bytes(b'3DSX' + bytes(1024))

    def tearDown(self):
        self.directory.cleanup()

    def test_exact_digest_and_length(self):
        text = release.manifest(self.binary, 'v1.2.3', 'https://github.com/owner/repo/releases/download/v1.2.3/game.3dsx')
        fields = dict(line.split('=', 1) for line in text.splitlines())
        self.assertEqual(fields['sha256'], hashlib.sha256(self.binary.read_bytes()).hexdigest())
        self.assertEqual(int(fields['size']), self.binary.stat().st_size)
        self.assertEqual(fields['version'], 'v1.2.3')

    def test_rejects_unsafe_input(self):
        for version, url in [('v1\nsize=1', 'https://github.com/file'), ('v1', 'http://github.com/file'), ('v1', 'https://user:pass@github.com/file')]:
            with self.subTest(version=version, url=url), self.assertRaises(ValueError):
                release.manifest(self.binary, version, url)

    def test_rejects_wrong_file(self):
        self.binary.write_bytes(b'<html>' + bytes(100))
        with self.assertRaises(ValueError):
            release.manifest(self.binary, 'v1', 'https://github.com/file')
