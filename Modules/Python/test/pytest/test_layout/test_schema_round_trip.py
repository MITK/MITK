# ============================================================================
#
# The Medical Imaging Interaction Toolkit (MITK)
#
# Copyright (c) German Cancer Research Center (DKFZ)
# All rights reserved.
#
# Use of this source code is governed by a 3-clause BSD license that can be
# found in the LICENSE file.
#
# ============================================================================

"""Integration-style schema and preset round-trip tests.

These tests are skipped in environments where the source tree is not on
disk (the wheel-test setup, for example) -- see ``conftest.py``. The
schema and preset paths are derived from the source layout so the DSL
stays in lockstep with the C++-side schema and presets.
"""

from __future__ import annotations

import json
from pathlib import Path

import pytest

from mitk.mxn.layout import MxNLayoutDocument, load_preset


def _has_jsonschema() -> bool:
    try:
        import jsonschema  # noqa: F401
    except ImportError:
        return False
    return True


class TestSchemaExamples:
    def test_every_example_validates_against_schema(self, schema_path):
        if schema_path is None:
            pytest.skip("schema fixture not available; run from source tree")
        if not _has_jsonschema():
            pytest.skip("jsonschema is not importable in this environment")
        import jsonschema

        with open(schema_path, encoding="utf-8") as f:
            schema = json.load(f)

        for i, example in enumerate(schema["examples"]):
            doc = MxNLayoutDocument.from_json(example)
            jsonschema.validate(doc.to_json(), schema)

    def test_every_example_passes_dsl_validate(self, schema_path):
        if schema_path is None:
            pytest.skip("schema fixture not available; run from source tree")
        with open(schema_path, encoding="utf-8") as f:
            schema = json.load(f)

        for example in schema["examples"]:
            doc = MxNLayoutDocument.from_json(example)
            doc.validate()


class TestInTreePreset:
    def test_two_rows_preset_round_trip(self, two_rows_preset_path):
        if two_rows_preset_path is None:
            pytest.skip("preset fixture not available; run from source tree")
        doc = load_preset(two_rows_preset_path)
        re_emitted = doc.to_json()
        with open(two_rows_preset_path, encoding="utf-8") as f:
            original = json.load(f)
        # Strict-mode equality with the in-tree preset confirms wire
        # compatibility with the engine's serializer.
        assert re_emitted == original

    def test_two_rows_preset_validates_against_schema(
        self, two_rows_preset_path, schema_path
    ):
        if two_rows_preset_path is None or schema_path is None:
            pytest.skip("source-tree fixtures not available")
        if not _has_jsonschema():
            pytest.skip("jsonschema is not importable in this environment")
        import jsonschema

        with open(schema_path, encoding="utf-8") as f:
            schema = json.load(f)
        doc = load_preset(two_rows_preset_path)
        jsonschema.validate(doc.to_json(), schema)


class TestMitkDataFixtures:
    """Validate the test-only v2 fixtures shipped in MITK-Data/MxNEditor.

    Intentionally schema-invalid fixtures (the view-direction typo, the
    v1.x migration-script inputs) are deliberately omitted -- they are
    exercised at the C++ / migration-script layer.
    """

    @pytest.mark.parametrize(
        "fixture_name",
        [
            "mxn_v2_lazy_mode.json",
            "mxn_v2_strict_missing_reference.json",
            "mxn_v2_duplicate_ids.json",
            "mxn_v2_seed_widget0_first.json",
            "mxn_v2_seed_widget1_first.json",
        ],
    )
    def test_fixture_validates_against_schema(
        self, data_dir, schema_path, fixture_name
    ):
        if schema_path is None:
            pytest.skip("schema fixture not available; run from source tree")
        if not _has_jsonschema():
            pytest.skip("jsonschema is not importable in this environment")
        import jsonschema

        fixture_path = data_dir / "MxNEditor" / fixture_name
        if not fixture_path.is_file():
            pytest.skip(f"MITK-Data fixture not available: {fixture_path}")

        with open(schema_path, encoding="utf-8") as f:
            schema = json.load(f)
        with open(fixture_path, encoding="utf-8") as f:
            fixture = json.load(f)
        jsonschema.validate(fixture, schema)
