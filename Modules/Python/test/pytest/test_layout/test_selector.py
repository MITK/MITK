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

"""Tests for ``MxNWindowSelector``."""

from __future__ import annotations

import pytest

from mitk.mxn.layout import (
    Group,
    LayoutWindow,
    LinkDimension,
    MxNLayoutDocument,
    MxNWindowSelector,
    Split,
    ViewDirection,
    grid,
)


class TestFiltering:
    def test_where_view_direction(self, fresh_doc):
        sel = fresh_doc.select_windows.where(view_direction=ViewDirection.AXIAL)
        assert sel.ids() == ["mxn__widget0", "mxn__widget3"]

    def test_where_view_direction_string(self, fresh_doc):
        sel = fresh_doc.select_windows.where(view_direction="axial")
        assert sel.ids() == ["mxn__widget0", "mxn__widget3"]

    def test_where_ids(self, fresh_doc):
        sel = fresh_doc.select_windows.where(ids=["mxn__widget1", "mxn__widget3"])
        assert sel.ids() == ["mxn__widget1", "mxn__widget3"]

    def test_where_display_names(self, fresh_doc):
        # Two of the four cells in the fixture have display labels.
        sel = fresh_doc.select_windows.where(display_names=["Top - Axial"])
        assert sel.ids() == ["mxn__widget0"]

    def test_where_display_names_none_matches_unset(self, fresh_doc):
        sel = fresh_doc.select_windows.where(display_names=[None])
        assert sel.ids() == ["mxn__widget2", "mxn__widget3"]

    def test_where_predicate(self, fresh_doc):
        sel = fresh_doc.select_windows.where(predicate=lambda w: w.id.endswith("0"))
        assert sel.ids() == ["mxn__widget0"]

    def test_where_group(self, fresh_doc):
        sel = fresh_doc.select_windows.where(group="row2")
        assert sel.ids() == ["mxn__widget2", "mxn__widget3"]

    def test_where_group_object(self, fresh_doc):
        sel = fresh_doc.select_windows.where(group=Group("row2"))
        assert sel.ids() == ["mxn__widget2", "mxn__widget3"]

    def test_chaining_is_and(self, fresh_doc):
        sel = (
            fresh_doc.select_windows
            .where(view_direction=ViewDirection.AXIAL)
            .where(group="row2")
        )
        assert sel.ids() == ["mxn__widget3"]

    def test_by_id_shorthand(self, fresh_doc):
        sel = fresh_doc.select_windows.by_id("mxn__widget0", "mxn__widget2")
        assert sel.ids() == ["mxn__widget0", "mxn__widget2"]

    def test_by_view_shorthand(self, fresh_doc):
        sel = fresh_doc.select_windows.by_view("axial")
        assert sel.ids() == ["mxn__widget0", "mxn__widget3"]


class TestLen:
    def test_len_iter(self, fresh_doc):
        sel = fresh_doc.select_windows.where(view_direction=ViewDirection.AXIAL)
        assert len(sel) == 2
        assert [w.id for w in sel] == ["mxn__widget0", "mxn__widget3"]


class TestUpdates:
    def test_link_to_string_group(self, fresh_doc):
        new_doc = (
            fresh_doc.select_windows
            .where(view_direction=ViewDirection.AXIAL)
            .link_to("row2")
        )
        # All axial cells now in row2; non-axial cells unchanged.
        for w in new_doc.windows():
            if w.view_direction is ViewDirection.AXIAL:
                assert w.selection == "row2"

    def test_link_to_unknown_group_materialises_default(self):
        a = LayoutWindow.create("mxn__a", "axial", selection="main")
        doc = MxNLayoutDocument.create(root=Split.horizontal(a))
        new_doc = doc.select_windows.by_id("mxn__a").link_to("brand_new")
        assert "brand_new" in new_doc.groups
        assert new_doc.groups["brand_new"].select_all is True

    def test_link_to_existing_group_preserves_select_all(self, fresh_doc):
        # row2 has select_all=False in the fixture; re-linking does not
        # overwrite the existing entry.
        new_doc = (
            fresh_doc.select_windows
            .by_id("mxn__widget0")
            .link_to("row2")
        )
        assert new_doc.groups["row2"].select_all is False

    def test_with_view_direction(self, fresh_doc):
        new_doc = (
            fresh_doc.select_windows
            .by_id("mxn__widget1")
            .with_view_direction(ViewDirection.AXIAL)
        )
        assert new_doc.find_window("mxn__widget1").view_direction is ViewDirection.AXIAL

    def test_with_display_name_string(self, fresh_doc):
        new_doc = (
            fresh_doc.select_windows
            .by_id("mxn__widget2")
            .with_display_name("Hello")
        )
        assert new_doc.find_window("mxn__widget2").name == "Hello"

    def test_with_display_name_callable(self, fresh_doc):
        new_doc = (
            fresh_doc.select_windows
            .where(view_direction=ViewDirection.AXIAL)
            .with_display_name(lambda w: f"Axial: {w.id}")
        )
        assert new_doc.find_window("mxn__widget0").name == "Axial: mxn__widget0"
        assert new_doc.find_window("mxn__widget3").name == "Axial: mxn__widget3"

    def test_with_display_name_none_clears(self, fresh_doc):
        new_doc = fresh_doc.select_windows.by_id("mxn__widget0").with_display_name(None)
        assert new_doc.find_window("mxn__widget0").name is None

    def test_map_escape_hatch(self, fresh_doc):
        from dataclasses import replace

        new_doc = fresh_doc.select_windows.by_id("mxn__widget0").map(
            lambda w: replace(w, size=5)
        )
        assert new_doc.find_window("mxn__widget0").size == 5

    def test_immutability_source_unchanged(self, fresh_doc):
        before_ids = fresh_doc.window_ids()
        before_groups = sorted(fresh_doc.groups.keys())
        _ = fresh_doc.select_windows.where(view_direction="axial").link_to("brand_new")
        # Source untouched.
        assert fresh_doc.window_ids() == before_ids
        assert sorted(fresh_doc.groups.keys()) == before_groups


class TestSplitSource:
    def test_split_select_windows_returns_split(self):
        s = grid(2, 2)
        relinked = s.select_windows.by_view("axial").link_to("row2")
        # No document -- return type matches input type.
        assert isinstance(relinked, Split)

    def test_split_source_no_group_materialisation(self):
        # On a Split source there is no group registry; link_to should still
        # work, it just doesn't have anywhere to materialise a Group.
        s = grid(1, 2)
        relinked = s.select_windows.by_id("mxn__widget0").link_to("brand_new")
        assert relinked.find_window("mxn__widget0").selection == "brand_new"
