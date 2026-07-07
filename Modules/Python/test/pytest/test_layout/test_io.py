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

"""Tests for `load_preset` / `save_preset`."""

from __future__ import annotations

import json
from pathlib import Path

import pytest

from mitk.mxn.layout import LayoutWindow, MxNLayoutDocument, Split, load_preset, save_preset


class TestRoundTrip:
    def test_strict_input_byte_equivalent(self, fresh_doc, tmp_path):
        path = tmp_path / "preset.json"
        save_preset(path, fresh_doc)
        recovered = load_preset(path)
        assert recovered == fresh_doc

    def test_save_accepts_raw_dict(self, fresh_doc, tmp_path):
        path = tmp_path / "raw.json"
        save_preset(path, fresh_doc.to_json())
        with open(path, encoding="utf-8") as f:
            on_disk = json.load(f)
        assert on_disk == fresh_doc.to_json()

    def test_lazy_input_materialises_to_strict(self, tmp_path):
        # A lazy-mode input file (no top-level `groups`) materialises to
        # strict during parse, so the round-trip is *equivalent* to the
        # original but adds an explicit `groups` block.
        lazy_path = tmp_path / "lazy.json"
        lazy_path.write_text(
            json.dumps(
                {
                    "version": "2.0",
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
            ),
            encoding="utf-8",
        )
        doc = load_preset(lazy_path)
        assert "main" in doc.groups
        # Re-emit and confirm groups is now explicit.
        strict_path = tmp_path / "strict.json"
        save_preset(strict_path, doc)
        with open(strict_path, encoding="utf-8") as f:
            on_disk = json.load(f)
        assert "groups" in on_disk
        # Re-loading the strict file gives an equal document.
        recovered = load_preset(strict_path)
        assert recovered == doc


class TestVersionMismatch:
    def test_loading_v3_file_raises(self, tmp_path):
        path = tmp_path / "v3.json"
        path.write_text(
            json.dumps(
                {
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
            ),
            encoding="utf-8",
        )
        with pytest.raises(ValueError, match="version"):
            load_preset(path)


class TestSaveTypes:
    def test_save_rejects_non_doc_non_mapping(self, tmp_path):
        with pytest.raises(ValueError, match="MxNLayoutDocument or Mapping"):
            save_preset(tmp_path / "x.json", "not a document")  # type: ignore[arg-type]
