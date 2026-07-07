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

"""Tests for the dataclass model: construction, normalisation, and the
hashable / equality contract.
"""

from __future__ import annotations

import pytest

from mitk.mxn.layout import (
    DEFAULT_GROUP,
    Group,
    LayoutWindow,
    Link,
    LinkDimension,
    MxNLayoutDocument,
    Split,
    ViewDirection,
)


class TestLayoutWindowCreate:
    def test_minimum_args_uses_defaults(self):
        w = LayoutWindow.create("mxn__widget0", ViewDirection.AXIAL)
        assert w.id == "mxn__widget0"
        assert w.view_direction is ViewDirection.AXIAL
        assert w.name is None
        assert w.size == 1
        assert w.selection == "main"
        assert w.links == {"selection": "main"}

    def test_string_view_direction_coerced(self):
        w = LayoutWindow.create("mxn__a", "axial")
        assert w.view_direction is ViewDirection.AXIAL

    def test_selection_accepts_group(self):
        g = Group("custom")
        w = LayoutWindow.create("mxn__a", "axial", selection=g)
        assert w.selection == "custom"

    def test_selection_accepts_link(self):
        w = LayoutWindow.create("mxn__a", "axial", selection=Link(group="custom"))
        # v2 normalises to bare-string storage.
        assert w.links == {"selection": "custom"}

    def test_selection_and_links_mutually_exclusive(self):
        with pytest.raises(ValueError, match="either 'selection' or 'links'"):
            LayoutWindow.create(
                "mxn__a",
                "axial",
                selection="main",
                links={"selection": "main"},
            )

    def test_links_must_include_selection(self):
        with pytest.raises(ValueError, match="selection"):
            LayoutWindow.create("mxn__a", "axial", links={"zoom": "main"})

    def test_bad_id_rejected_by_create(self):
        with pytest.raises(ValueError, match="qualified-id pattern"):
            LayoutWindow.create("widget0", "axial")

    def test_empty_display_name_rejected(self):
        with pytest.raises(ValueError, match="non-empty"):
            LayoutWindow.create("mxn__a", "axial", name="")

    def test_size_must_be_positive(self):
        with pytest.raises(ValueError, match="size"):
            LayoutWindow.create("mxn__a", "axial", size=0)

    def test_bool_size_rejected(self):
        with pytest.raises(ValueError, match="size"):
            LayoutWindow.create("mxn__a", "axial", size=True)


class TestLayoutWindowDirectInit:
    """The bare dataclass init is the *private* form: it only normalises
    storage, not validates input. Documents this contract explicitly so
    we notice if it changes.
    """

    def test_direct_init_skips_id_validation(self):
        # Direct construction does NOT raise on a bad id; this is by
        # design (validation lives in `create()`). MxNLayoutDocument.validate
        # is the second line of defence for cross-document checks.
        bad = LayoutWindow(
            id="this-is-not-qualified",
            view_direction=ViewDirection.AXIAL,
            _links=(("selection", "main"),),
        )
        assert bad.id == "this-is-not-qualified"

    def test_direct_init_sorts_links(self):
        unsorted = LayoutWindow(
            id="mxn__a",
            view_direction=ViewDirection.AXIAL,
            _links=(("zoom", "z"), ("selection", "main")),
        )
        # Sorted by dimension name for hashability and equality stability.
        assert unsorted._links == (("selection", "main"), ("zoom", "z"))


class TestLayoutWindowTransformations:
    def test_with_link_replaces_existing_dimension(self):
        w = LayoutWindow.create("mxn__a", "axial")
        w2 = w.with_link("selection", "row2")
        assert w2.selection == "row2"
        assert w.selection == "main"  # original untouched

    def test_with_id_validates_new_id(self):
        w = LayoutWindow.create("mxn__a", "axial")
        with pytest.raises(ValueError, match="qualified-id pattern"):
            w.with_id("bare")

    def test_with_name_clears_when_none(self):
        w = LayoutWindow.create("mxn__a", "axial", name="Hello")
        cleared = w.with_name(None)
        assert cleared.name is None

    def test_linked_to_accepts_str_and_group(self):
        w = LayoutWindow.create("mxn__a", "axial", selection="main")
        assert w.linked_to("main")
        assert w.linked_to(Group("main"))
        assert not w.linked_to("row2")


class TestSplit:
    def test_horizontal_vararg(self):
        a = LayoutWindow.create("mxn__a", "axial")
        b = LayoutWindow.create("mxn__b", "sagittal")
        split = Split.horizontal(a, b)
        assert split.orientation == "horizontal"
        assert split.children == (a, b)

    def test_empty_children_rejected(self):
        with pytest.raises(ValueError, match="at least one"):
            Split(orientation="horizontal", children=())

    def test_bad_orientation_rejected(self):
        a = LayoutWindow.create("mxn__a", "axial")
        with pytest.raises(ValueError, match="orientation"):
            Split(orientation="diagonal", children=(a,))  # type: ignore[arg-type]

    def test_pre_order_iteration(self):
        a = LayoutWindow.create("mxn__a", "axial")
        b = LayoutWindow.create("mxn__b", "sagittal")
        c = LayoutWindow.create("mxn__c", "coronal")
        split = Split.vertical(Split.horizontal(a, b), c)
        # Pre-order: a, b, c.
        assert split.window_ids() == ["mxn__a", "mxn__b", "mxn__c"]

    def test_find_window(self):
        a = LayoutWindow.create("mxn__a", "axial")
        b = LayoutWindow.create("mxn__b", "sagittal")
        split = Split.horizontal(a, b)
        assert split.find_window("mxn__a") is a
        assert split.find_window("nope") is None


class TestMxNLayoutDocumentCreate:
    def test_groups_auto_materialised_when_omitted(self):
        a = LayoutWindow.create("mxn__a", "axial", selection="custom")
        doc = MxNLayoutDocument.create(root=Split.horizontal(a))
        assert "custom" in doc.groups
        assert doc.groups["custom"].select_all is True

    def test_groups_mapping_key_must_match(self):
        a = LayoutWindow.create("mxn__a", "axial", selection="main")
        with pytest.raises(ValueError, match="does not match"):
            MxNLayoutDocument.create(
                root=Split.horizontal(a),
                groups={"renamed": Group("main")},
            )

    def test_groups_iterable_uniqueness(self):
        a = LayoutWindow.create("mxn__a", "axial", selection="main")
        with pytest.raises(ValueError, match="duplicate"):
            MxNLayoutDocument.create(
                root=Split.horizontal(a),
                groups=[Group("main"), Group("main", select_all=False)],
            )

    def test_groups_string_rejected(self):
        # `str` is technically Iterable; iterating it yields characters and
        # produces a confusing downstream error. Caught at the boundary.
        a = LayoutWindow.create("mxn__a", "axial", selection="main")
        with pytest.raises(ValueError, match="Mapping"):
            MxNLayoutDocument.create(
                root=Split.horizontal(a),
                groups="main",  # type: ignore[arg-type]
            )

    def test_unknown_group_in_window_raises(self):
        a = LayoutWindow.create("mxn__a", "axial", selection="ghost")
        with pytest.raises(ValueError, match="unknown group"):
            MxNLayoutDocument.create(
                root=Split.horizontal(a),
                groups={"main": Group("main")},
            )

    def test_duplicate_window_ids_rejected(self):
        a = LayoutWindow.create("mxn__a", "axial")
        b = LayoutWindow.create("mxn__a", "sagittal")  # same id
        with pytest.raises(ValueError, match="duplicate"):
            MxNLayoutDocument.create(root=Split.horizontal(a, b))

    def test_groups_property_is_read_only_view(self, fresh_doc):
        view = fresh_doc.groups
        with pytest.raises(TypeError):
            view["new"] = Group("new")  # type: ignore[index]


class TestMxNLayoutDocumentRenumber:
    def test_with_default_ids_preorder(self):
        a = LayoutWindow.create("mxn__alpha", "axial", name="A")
        b = LayoutWindow.create("mxn__beta", "sagittal")
        c = LayoutWindow.create("mxn__gamma", "coronal", name="C")
        doc = MxNLayoutDocument.create(
            root=Split.vertical(Split.horizontal(a, b), c)
        )
        renumbered = doc.with_default_ids()
        assert renumbered.window_ids() == [
            "mxn__widget0",
            "mxn__widget1",
            "mxn__widget2",
        ]
        # Display names preserved.
        assert renumbered.find_window("mxn__widget0").name == "A"
        assert renumbered.find_window("mxn__widget1").name is None
        assert renumbered.find_window("mxn__widget2").name == "C"

    def test_with_default_ids_custom_editor_and_stem(self):
        a = LayoutWindow.create("mxn__alpha", "axial")
        doc = MxNLayoutDocument.create(root=Split.horizontal(a))
        renumbered = doc.with_default_ids(editor_name="other", bare_id_stem="cell")
        assert renumbered.window_ids() == ["other__cell0"]

    def test_with_default_ids_start_index(self):
        a = LayoutWindow.create("mxn__alpha", "axial")
        b = LayoutWindow.create("mxn__beta", "sagittal")
        doc = MxNLayoutDocument.create(root=Split.horizontal(a, b))
        renumbered = doc.with_default_ids(start_index=10)
        assert renumbered.window_ids() == ["mxn__widget10", "mxn__widget11"]


class TestEqualityAndHashing:
    def test_equality_across_construction_routes(self, fresh_doc):
        round_trip = MxNLayoutDocument.from_json(fresh_doc.to_json())
        assert round_trip == fresh_doc

    def test_doc_hashable_as_dict_key(self, fresh_doc):
        d = {fresh_doc: "value"}
        assert d[fresh_doc] == "value"
        assert len({fresh_doc, fresh_doc}) == 1

    def test_window_hashable(self):
        w = LayoutWindow.create("mxn__a", "axial")
        s = {w, w}
        assert len(s) == 1

    def test_split_hashable(self):
        a = LayoutWindow.create("mxn__a", "axial")
        s = Split.horizontal(a)
        assert len({s, s}) == 1

    def test_link_hashable(self):
        assert len({Link(group="main"), Link(group="main")}) == 1

    def test_group_hashable(self):
        assert len({Group("main"), Group("main")}) == 1

    def test_link_storage_order_insensitive_equality(self):
        a = LayoutWindow(
            id="mxn__a",
            view_direction=ViewDirection.AXIAL,
            _links=(("selection", "main"), ("zoom", "z")),
        )
        b = LayoutWindow(
            id="mxn__a",
            view_direction=ViewDirection.AXIAL,
            _links=(("zoom", "z"), ("selection", "main")),
        )
        assert a == b
        assert hash(a) == hash(b)


class TestLinks:
    def test_links_returns_fresh_dict(self):
        w = LayoutWindow.create("mxn__a", "axial")
        d = w.links
        d["selection"] = "ghost"  # mutate the returned dict
        # Original is unchanged because the property returns a copy.
        assert w.selection == "main"

    def test_selection_property_for_v3_link_value(self):
        # Force-store a Link object via direct init (private path) and
        # verify the selection property unwraps it correctly. v2 callers
        # never see this shape on storage; v3 may.
        w = LayoutWindow(
            id="mxn__a",
            view_direction=ViewDirection.AXIAL,
            _links=(("selection", Link(group="main")),),
        )
        assert w.selection == "main"


class TestDefaultGroup:
    def test_default_group_is_main_with_select_all(self):
        assert DEFAULT_GROUP.name == "main"
        assert DEFAULT_GROUP.select_all is True
