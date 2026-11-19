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

"""Tests for the validation rules.

Per-window rules are enforced once in `LayoutWindow.create`; cross-document
rules live on `MxNLayoutDocument.validate()`. The split is the test matrix.
"""

from __future__ import annotations

import pytest

from mitk.mxn.layout import (
    Group,
    LayoutWindow,
    MxNLayoutDocument,
    Split,
    ViewDirection,
    grid,
)


class TestPerWindowRulesInCreate:
    @pytest.mark.parametrize(
        "bad_id",
        [
            "widget0",  # missing __ delimiter
            "__widget0",  # empty editor segment
            "_x__widget0",  # editor segment cannot start with `_`
            "mxn__",  # empty bare-id segment
            "1mxn__widget0",  # editor segment must start with a letter
        ],
    )
    def test_bad_ids_rejected(self, bad_id):
        with pytest.raises(ValueError, match="qualified-id pattern"):
            LayoutWindow.create(bad_id, "axial")

    def test_empty_name_rejected(self):
        with pytest.raises(ValueError, match="non-empty"):
            LayoutWindow.create("mxn__a", "axial", name="")

    def test_unknown_view_direction_rejected(self):
        with pytest.raises(ValueError):
            LayoutWindow.create("mxn__a", "diagonal")

    def test_size_must_be_positive(self):
        with pytest.raises(ValueError, match="size"):
            LayoutWindow.create("mxn__a", "axial", size=0)

    def test_invalid_group_name_in_links(self):
        with pytest.raises(ValueError, match="group name"):
            LayoutWindow.create("mxn__a", "axial", selection="bad name with spaces")

    def test_required_selection_dimension(self):
        with pytest.raises(ValueError, match="selection"):
            LayoutWindow.create("mxn__a", "axial", links={"zoom": "main"})


class TestCrossDocumentRules:
    def test_duplicate_ids_caught(self):
        a = LayoutWindow.create("mxn__a", "axial")
        b = LayoutWindow.create("mxn__a", "sagittal")
        with pytest.raises(ValueError, match="duplicate window id"):
            MxNLayoutDocument.create(root=Split.horizontal(a, b))

    def test_unknown_group_reference_caught(self):
        a = LayoutWindow.create("mxn__a", "axial", selection="ghost")
        with pytest.raises(ValueError, match="unknown group"):
            MxNLayoutDocument.create(
                root=Split.horizontal(a),
                groups={"main": Group("main")},
            )

    def test_invalid_group_name_in_groups(self):
        a = LayoutWindow.create("mxn__a", "axial")
        # The Group dataclass itself doesn't validate; the document does.
        with pytest.raises(ValueError, match="group name"):
            MxNLayoutDocument.create(
                root=Split.horizontal(a),
                groups={"bad name": Group("bad name")},
            )

    def test_explicit_wrong_version_raises(self):
        a = LayoutWindow.create("mxn__a", "axial")
        with pytest.raises(ValueError, match="version"):
            MxNLayoutDocument.create(root=Split.horizontal(a), version="3.0")


class TestDeliberateNonChecks:
    """The DSL is editor-instance-agnostic. These cases pass DSL validation
    but the C++ engine's apply-time check rejects them. Documenting via test
    so a future reader doesn't accidentally tighten the surface.
    """

    def test_non_default_editor_prefix_accepted(self):
        # `other__widget0` matches the qualified pattern; the engine
        # rejects it at apply time when the editor's `multiWidgetName` is
        # `mxn`. The DSL has no editor instance to compare against.
        w = LayoutWindow.create("other__widget0", "axial")
        doc = MxNLayoutDocument.create(root=Split.horizontal(w))
        doc.validate()  # passes

    def test_mixed_editor_prefixes_in_one_doc_accepted(self):
        a = LayoutWindow.create("mxn__a", "axial")
        b = LayoutWindow.create("other__b", "axial")
        doc = MxNLayoutDocument.create(root=Split.horizontal(a, b))
        doc.validate()  # passes; engine would reject at apply time


_RESERVED_SPELLINGS = ["global", "Global", "GLOBAL"]


def _v2_document(selection="main", groups=None):
    """A one-window 2.0 document as raw JSON."""
    return {
        "version": "2.0",
        "root": {
            "type": "split",
            "orientation": "horizontal",
            "children": [
                {"type": "window", "id": "mxn__a", "view_direction": "axial", "links": {"selection": selection}}
            ],
        },
        "groups": groups if groups is not None else {selection: {"select_all": True}},
    }


def _plain_window():
    return LayoutWindow.create("mxn__a", "axial")


_ENTRY_POINTS = {
    "create selection": lambda name: LayoutWindow.create("mxn__a", "axial", selection=name),
    "create links": lambda name: LayoutWindow.create("mxn__a", "axial", links={"selection": "main", "zoom": name}),
    "with_link": lambda name: _plain_window().with_link("selection", name),
    "grid": lambda name: grid(1, 1, group=name),
    "groups mapping": lambda name: MxNLayoutDocument.create(
        root=Split.horizontal(_plain_window()), groups={"main": Group("main"), name: Group(name)}
    ),
    "groups iterable": lambda name: MxNLayoutDocument.create(
        root=Split.horizontal(_plain_window()), groups=[Group("main"), Group(name)]
    ),
    "from_json groups key": lambda name: MxNLayoutDocument.from_json(
        _v2_document(groups={"main": {"select_all": True}, name: {"select_all": True}})
    ),
    "from_json selection": lambda name: MxNLayoutDocument.from_json(_v2_document(selection=name)),
}


class TestReservedGroup:
    """The engine reserves the group id `global`, in every spelling, for the
    group that follows the application's value, and refuses it in 2.0
    documents. The DSL writes 2.0, so it refuses the id wherever a group
    name enters.
    """

    @pytest.mark.parametrize("name", _RESERVED_SPELLINGS)
    @pytest.mark.parametrize("entry", list(_ENTRY_POINTS))
    def test_reserved_group_rejected(self, entry, name):
        with pytest.raises(ValueError, match="reserved"):
            _ENTRY_POINTS[entry](name)

    @pytest.mark.parametrize("name", _RESERVED_SPELLINGS)
    def test_link_to_rejects_reserved_group(self, fresh_doc, name):
        with pytest.raises(ValueError, match="reserved"):
            fresh_doc.select_windows.link_to(name)

    @pytest.mark.parametrize("name", ["globals", "global2", "g_global", "my.global"])
    def test_lookalikes_accepted(self, name):
        window = LayoutWindow.create("mxn__a", "axial", selection=name)
        MxNLayoutDocument.create(root=Split.horizontal(window)).validate()
