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

"""Tests for JSON round-trip and the up-front version check."""

from __future__ import annotations

import json
from pathlib import Path

import pytest

from mitk.mxn.layout import (
    LAYOUT_VERSION,
    LayoutWindow,
    MxNLayoutDocument,
    Split,
    ViewDirection,
)


def _read_examples(schema_path: Path) -> list[dict]:
    with open(schema_path, encoding="utf-8") as f:
        return json.load(f)["examples"]


class TestJsonRoundTripFromSchemaExamples:
    def test_every_schema_example_round_trips(self, schema_path):
        if schema_path is None:
            pytest.skip("schema fixture not available; run from source tree")
        for example in _read_examples(schema_path):
            doc = MxNLayoutDocument.from_json(example)
            re_emitted = doc.to_json()
            doc2 = MxNLayoutDocument.from_json(re_emitted)
            assert doc == doc2

    def test_lazy_input_materialises_to_strict_on_parse(self, schema_path):
        if schema_path is None:
            pytest.skip("schema fixture not available; run from source tree")
        examples = _read_examples(schema_path)
        # Example index 2 in the in-tree schema has no top-level `groups`.
        lazy = next(ex for ex in examples if "groups" not in ex)
        parsed = MxNLayoutDocument.from_json(lazy)
        # In-memory always strict: every referenced label has a Group entry.
        for window in parsed.windows():
            assert window.selection in parsed.groups

    def test_strict_output_emits_groups_explicitly(self, schema_path):
        if schema_path is None:
            pytest.skip("schema fixture not available; run from source tree")
        examples = _read_examples(schema_path)
        lazy = next(ex for ex in examples if "groups" not in ex)
        emitted = MxNLayoutDocument.from_json(lazy).to_json()
        assert "groups" in emitted


class TestUpFrontVersionCheck:
    def test_v3_versioned_document_rejected(self):
        future_doc = {
            "version": "3.0",
            "groups": {"main": {"select_all": True}},
            "root": {
                "type": "split",
                "orientation": "horizontal",
                "children": [
                    {
                        "type": "window",
                        "id": "mxn__widget0",
                        "view_direction": "axial",
                        "links": {"selection": "main"},
                    }
                ],
            },
        }
        with pytest.raises(ValueError, match="version"):
            MxNLayoutDocument.from_json(future_doc)

    def test_missing_version_field_rejected(self):
        with pytest.raises(ValueError, match="version"):
            MxNLayoutDocument.from_json({
                "root": {
                    "type": "split",
                    "orientation": "horizontal",
                    "children": [],
                }
            })


class TestKeyOrdering:
    def test_top_level_key_order(self, fresh_doc):
        keys = list(fresh_doc.to_json().keys())
        assert keys == ["version", "name", "groups", "root"]

    def test_window_key_order_with_name(self):
        w = LayoutWindow.create("mxn__a", "axial", name="Display")
        doc = MxNLayoutDocument.create(root=Split.horizontal(w))
        emitted = doc.to_json()
        window_dict = emitted["root"]["children"][0]
        assert list(window_dict.keys()) == [
            "type",
            "id",
            "name",
            "view_direction",
            "links",
            "size",
        ]

    def test_window_key_order_without_name(self):
        w = LayoutWindow.create("mxn__a", "axial")
        doc = MxNLayoutDocument.create(root=Split.horizontal(w))
        emitted = doc.to_json()
        window_dict = emitted["root"]["children"][0]
        assert list(window_dict.keys()) == [
            "type",
            "id",
            "view_direction",
            "links",
            "size",
        ]

    def test_root_split_omits_size(self, fresh_doc):
        emitted = fresh_doc.to_json()
        assert "size" not in emitted["root"]

    def test_inner_split_emits_size(self, fresh_doc):
        emitted = fresh_doc.to_json()
        # The fixture root is a vertical split with two horizontal children;
        # inner splits emit `size`.
        for child in emitted["root"]["children"]:
            if child["type"] == "split":
                assert "size" in child


class TestNameRoundTrip:
    def test_name_present_round_trips(self):
        w = LayoutWindow.create("mxn__a", "axial", name="Hello")
        doc = MxNLayoutDocument.create(root=Split.horizontal(w))
        recovered = MxNLayoutDocument.from_json(doc.to_json())
        assert recovered.find_window("mxn__a").name == "Hello"

    def test_name_absent_round_trips(self):
        w = LayoutWindow.create("mxn__a", "axial")
        doc = MxNLayoutDocument.create(root=Split.horizontal(w))
        recovered = MxNLayoutDocument.from_json(doc.to_json())
        assert recovered.find_window("mxn__a").name is None


class TestParseFromString:
    def test_from_json_accepts_string(self, fresh_doc):
        text = json.dumps(fresh_doc.to_json())
        recovered = MxNLayoutDocument.from_json(text)
        assert recovered == fresh_doc

    def test_invalid_json_string_rejected(self):
        with pytest.raises(ValueError, match="JSON"):
            MxNLayoutDocument.from_json("{not json")
