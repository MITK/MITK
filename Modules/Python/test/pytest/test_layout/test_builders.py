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

"""Tests for the split-returning builders."""

from __future__ import annotations

import pytest

from mitk.mxn.layout import (
    Group,
    LayoutWindow,
    MxNLayoutDocument,
    Split,
    ViewDirection,
    grid,
    single_window,
    three_up,
    two_rows_each_direction,
)


class TestGrid:
    def test_2x3_default_view_directions(self):
        s = grid(2, 3)
        ids = s.window_ids()
        assert ids == [f"mxn__widget{i}" for i in range(6)]
        # Default view direction cycle is axial / sagittal / coronal.
        views = [w.view_direction for w in s.windows()]
        assert views[0] is ViewDirection.AXIAL
        assert views[1] is ViewDirection.SAGITTAL
        assert views[2] is ViewDirection.CORONAL
        assert views[3] is ViewDirection.AXIAL  # cycle restarts on row 2

    def test_grid_with_display_names(self):
        s = grid(1, 3, display_names=["A", None, "C"])
        names = [w.name for w in s.windows()]
        assert names == ["A", None, "C"]

    def test_grid_display_names_length_mismatch(self):
        with pytest.raises(ValueError, match="display_names"):
            grid(2, 3, display_names=["A", "B"])

    def test_grid_explicit_views(self):
        s = grid(1, 2, view_directions=[ViewDirection.CORONAL, ViewDirection.AXIAL])
        views = [w.view_direction for w in s.windows()]
        assert views == [ViewDirection.CORONAL, ViewDirection.AXIAL]

    def test_grid_views_length_mismatch(self):
        with pytest.raises(ValueError, match="view_directions"):
            grid(2, 2, view_directions=[ViewDirection.AXIAL])

    def test_grid_id_compose_via_start_index(self):
        first = grid(1, 2)
        second = grid(1, 2, start_index=2)
        assert first.window_ids() == ["mxn__widget0", "mxn__widget1"]
        assert second.window_ids() == ["mxn__widget2", "mxn__widget3"]

    def test_grid_custom_editor_and_stem(self):
        s = grid(1, 2, editor_name="other", bare_id_stem="cell")
        assert s.window_ids() == ["other__cell0", "other__cell1"]

    def test_grid_with_group_object(self):
        s = grid(1, 2, group=Group("custom"))
        for w in s.windows():
            assert w.selection == "custom"

    def test_grid_collapses_single_row(self):
        s = grid(1, 3)
        assert s.orientation == "horizontal"
        assert len(s.children) == 3
        assert all(isinstance(c, LayoutWindow) for c in s.children)

    def test_grid_collapses_single_column(self):
        s = grid(3, 1)
        assert s.orientation == "vertical"
        assert all(isinstance(c, LayoutWindow) for c in s.children)

    def test_grid_rejects_zero_rows(self):
        with pytest.raises(ValueError, match="rows"):
            grid(0, 3)

    def test_grid_rejects_zero_cols(self):
        with pytest.raises(ValueError, match="cols"):
            grid(3, 0)


class TestThreeUp:
    def test_shape(self):
        s = three_up()
        assert s.orientation == "horizontal"
        assert len(s.children) == 3
        views = [w.view_direction for w in s.windows()]
        assert views == [ViewDirection.AXIAL, ViewDirection.SAGITTAL, ViewDirection.CORONAL]


class TestTwoRowsEachDirection:
    def test_shape_matches_in_tree_preset(self):
        s = two_rows_each_direction()
        assert s.orientation == "vertical"
        assert len(s.children) == 2
        # Six windows, top row links to "main", bottom to "row2".
        windows = list(s.windows())
        assert len(windows) == 6
        assert all(w.selection == "main" for w in windows[:3])
        assert all(w.selection == "row2" for w in windows[3:])
        assert s.window_ids() == [f"mxn__widget{i}" for i in range(6)]


class TestSingleWindow:
    def test_default_synthesises_qualified_id(self):
        s = single_window()
        assert s.orientation == "horizontal"
        assert len(s.children) == 1
        assert s.children[0].id == "mxn__widget0"

    def test_explicit_id_qualified(self):
        s = single_window(id_qualified="mxn__alpha")
        assert s.children[0].id == "mxn__alpha"

    def test_bare_id_qualified_rejected(self):
        # `id_qualified` is named to make the contract loud -- passing a bare id
        # raises from the regex check inside `LayoutWindow.create`.
        with pytest.raises(ValueError, match="qualified-id pattern"):
            single_window(id_qualified="alpha")

    def test_display_name_propagates(self):
        s = single_window(display_name="The Window")
        assert s.children[0].name == "The Window"


class TestRenumberAfterCompose:
    def test_with_default_ids_after_handstitched_compose(self):
        a = LayoutWindow.create(id="mxn__alpha", view_direction="axial", name="Keep me")
        b = LayoutWindow.create(id="mxn__beta", view_direction="sagittal")
        composed = Split.horizontal(a, b)
        doc = MxNLayoutDocument.create(root=composed)
        renumbered = doc.with_default_ids()
        assert renumbered.window_ids() == ["mxn__widget0", "mxn__widget1"]
        assert renumbered.find_window("mxn__widget0").name == "Keep me"
