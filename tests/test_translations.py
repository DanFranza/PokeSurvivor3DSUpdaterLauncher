import importlib.util
import json
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('translations', ROOT / 'tools/generate_translations.py')
translations = importlib.util.module_from_spec(spec)
spec.loader.exec_module(translations)


class TranslationTests(unittest.TestCase):
    def test_all_five_columns_and_placeholders(self):
        rows = translations.catalog()
        self.assertGreaterEqual(len(rows), 120)
        self.assertEqual(len(rows), len({row[0] for row in rows}))
        self.assertEqual((ROOT / 'source/translations.inc').read_text(), translations.generated())

    def test_all_static_errors_have_translations(self):
        keys = {row[0] for row in translations.catalog()}
        for name in ['storage.cpp', 'update_source.cpp', 'network.cpp', 'launch.cpp', 'cia.cpp', 'main.cpp']:
            text = (ROOT / 'source' / name).read_text()
            for literal in re.findall(r'\b(?:e|error)\s*=\s*("(?:\\.|[^"\\])*")', text):
                value = json.loads(literal)
                with self.subTest(file=name, message=value):
                    self.assertIn(value, keys)

    def test_ui_keys_and_network_stages_have_translations(self):
        keys = {row[0] for row in translations.catalog()}
        technical = {'POKÉ', 'SURVIVOR', 'X', '▶', '★', 'CIA', '3DSX'}
        for name in ['main.cpp', 'network.cpp', 'launch.cpp', 'cia.cpp']:
            text = (ROOT / 'source' / name).read_text()
            for literal in re.findall(r'\b(?:dialog|frame|progress|text|tr|trf|failure|failed)\s*\(\s*("(?:\\.|[^"\\])*")', text):
                value = json.loads(literal)
                if value in technical:
                    continue
                with self.subTest(file=name, message=value):
                    self.assertIn(value, keys)
