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

"""The layout presets MitkQtWidgets ships validate against the v3 schema.

The presets are read as raw JSON: they are 3.0 documents, and the DSL
targets 2.0. Nothing here imports MITK.
"""

from __future__ import annotations

import json

import pytest


def test_every_shipped_preset_validates_against_v3_schema(v3_schema_path):
    if v3_schema_path is None:
        pytest.skip("v3 schema fixture not available; run from source tree")
    jsonschema = pytest.importorskip("jsonschema")

    with open(v3_schema_path, encoding="utf-8") as f:
        schema = json.load(f)

    presets = sorted(v3_schema_path.parent.glob("mxnLayout_*.json"))
    assert presets, f"no mxnLayout_*.json preset found beside {v3_schema_path}"

    failures = []
    for preset in presets:
        with open(preset, encoding="utf-8") as f:
            document = json.load(f)
        try:
            jsonschema.validate(document, schema)
        except jsonschema.ValidationError as e:
            failures.append(f"{preset.name}: {e.message} at {list(e.absolute_path)}")
    assert not failures, "\n".join(failures)
