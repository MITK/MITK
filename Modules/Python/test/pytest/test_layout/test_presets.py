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

"""Tests for the named preset registry."""

from __future__ import annotations

import pytest

from mitk.mxn.layout import PRESETS, MxNLayoutDocument, list_presets, preset


class TestRegistry:
    def test_known_presets_listed(self):
        names = list_presets()
        assert "three-up" in names
        assert "two-rows-each-direction" in names
        assert "single" in names

    def test_lookup_returns_validated_document(self):
        for name in list_presets():
            doc = preset(name)
            assert isinstance(doc, MxNLayoutDocument)
            doc.validate()  # already validated by `create`; defensive double-check.

    def test_unknown_preset_raises_keyerror_listing_available(self):
        with pytest.raises(KeyError) as excinfo:
            preset("nonexistent")
        message = str(excinfo.value)
        for known in list_presets():
            assert known in message

    def test_each_call_returns_independent_document(self):
        a = preset("three-up")
        b = preset("three-up")
        # Equal but not the same object (factory invocation per call).
        assert a == b
        assert a is not b

    def test_PRESETS_keys_match_list_presets(self):
        assert sorted(PRESETS.keys()) == list_presets()
