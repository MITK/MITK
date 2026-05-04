#!/usr/bin/env python3
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
"""Unit tests for `migrate-mxn-layout-v1-to-v2.py`.

The migration script self-validates its output against the v2 schema when
`jsonschema` is available, which catches structural drift. These tests pin
the *behavioural* contract that schema validation cannot:

- `synchGroup` vs `syncGroup` precedence and divergence warning
- First-encounter-wins for `selectAll` across cells in the same group
- Group integer -> label convention (`1 -> main`, `N -> g_N`)
- Pre-order `widget<i>` numbering across nested splits
- GCD reduction of v1 pixel-derived sibling sizes
- Auto-wrapping of a single-window v1 root
- Editor-name flag plumbing into the qualified `id` form

Exit codes match the surrounding CTest convention: 0 on success, non-zero
on test failure. Skipping is left to CTest's `SKIP_RETURN_CODE` machinery
when invoked via the build system.
"""

from __future__ import annotations

import importlib.util
import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path


SCRIPT_PATH = (
    Path(__file__).resolve().parent.parent / "resource" / "migrate-mxn-layout-v1-to-v2.py"
)


def _load_migration_module():
    """Load the migration script as a module despite its dashed filename."""
    spec = importlib.util.spec_from_file_location("mxn_migration", SCRIPT_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load migration script from {SCRIPT_PATH}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


migration = _load_migration_module()


def _v1_window(view_direction="axial", group=1, select_all=True, size=None):
    """Build a v1 window node fragment."""
    out = {
        "isWindow": True,
        "viewDirection": view_direction,
        "synchGroup": group,
        "selectAll": select_all,
    }
    if size is not None:
        out["size"] = size
    return out


def _v1_split(children, vertical=False, size=None):
    """Build a v1 split node fragment."""
    out = {
        "isWindow": False,
        "vertical": vertical,
        "content": children,
    }
    if size is not None:
        out["size"] = size
    return out


def _v1_doc(root, name=None):
    """Wrap a converted v1 root with the minimum metadata `migrate()` expects."""
    out = {"version": "1.0"}
    if name is not None:
        out["name"] = name
    # The top-level v1 doc IS the root node; merge the root fields into it.
    merged = dict(out)
    merged.update(root)
    return merged


class GroupHandlingTests(unittest.TestCase):
    """`synchGroup`/`syncGroup` precedence and the diverge warning."""

    def test_synchGroup_typo_takes_precedence_over_syncGroup(self):
        node = {
            "isWindow": True,
            "viewDirection": "axial",
            "synchGroup": 1,
            "syncGroup": 2,
        }
        captured = io.StringIO()
        with redirect_stderr(captured):
            value = migration._read_group_int(node, where="$")
        self.assertEqual(value, 1)
        self.assertIn("warning:", captured.getvalue())
        self.assertIn("synchGroup", captured.getvalue())

    def test_synchGroup_only(self):
        value = migration._read_group_int({"synchGroup": 3}, where="$")
        self.assertEqual(value, 3)

    def test_syncGroup_only(self):
        value = migration._read_group_int({"syncGroup": 4}, where="$")
        self.assertEqual(value, 4)

    def test_neither_defaults_to_one_with_warning(self):
        captured = io.StringIO()
        with redirect_stderr(captured):
            value = migration._read_group_int({}, where="$")
        self.assertEqual(value, 1)
        self.assertIn("warning:", captured.getvalue())

    def test_group_label_convention(self):
        self.assertEqual(migration._group_label(1), "main")
        self.assertEqual(migration._group_label(2), "g_2")
        self.assertEqual(migration._group_label(7), "g_7")


class SelectAllFirstEncounterTests(unittest.TestCase):
    """First-encounter-wins for `selectAll` across cells in the same group."""

    def test_first_cell_in_group_pins_selectAll(self):
        v1 = _v1_doc(
            _v1_split(
                [
                    _v1_window(group=1, select_all=False),
                    _v1_window(view_direction="sagittal", group=1, select_all=True),
                ]
            )
        )
        captured = io.StringIO()
        with redirect_stderr(captured):
            v2 = migration.migrate(v1)
        self.assertFalse(v2["groups"]["main"]["select_all"])
        # Diverging follow-up should warn, but the first-encounter value wins.
        self.assertIn("first-encounter value wins", captured.getvalue())

    def test_consistent_selectAll_no_warning(self):
        v1 = _v1_doc(
            _v1_split(
                [
                    _v1_window(group=2, select_all=True),
                    _v1_window(view_direction="coronal", group=2, select_all=True),
                ]
            )
        )
        captured = io.StringIO()
        with redirect_stderr(captured):
            v2 = migration.migrate(v1)
        self.assertTrue(v2["groups"]["g_2"]["select_all"])
        self.assertNotIn("first-encounter", captured.getvalue())


class WidgetNumberingTests(unittest.TestCase):
    """Pre-order `widget<i>` numbering across nested splits."""

    def test_pre_order_numbering_in_nested_splits(self):
        # Two rows of three windows each - widget0..widget5 in pre-order.
        v1 = _v1_doc(
            _v1_split(
                [
                    _v1_split(
                        [
                            _v1_window(view_direction="axial"),
                            _v1_window(view_direction="sagittal"),
                            _v1_window(view_direction="coronal"),
                        ]
                    ),
                    _v1_split(
                        [
                            _v1_window(view_direction="axial", group=2),
                            _v1_window(view_direction="sagittal", group=2),
                            _v1_window(view_direction="coronal", group=2),
                        ]
                    ),
                ],
                vertical=True,
            )
        )
        v2 = migration.migrate(v1)
        # Walk the tree and collect ids in pre-order.
        ids = []

        def collect(node):
            if node["type"] == "window":
                ids.append(node["id"])
                return
            for child in node["children"]:
                collect(child)

        collect(v2["root"])
        self.assertEqual(
            ids,
            [
                "mxn__widget0",
                "mxn__widget1",
                "mxn__widget2",
                "mxn__widget3",
                "mxn__widget4",
                "mxn__widget5",
            ],
        )

    def test_editor_name_flag_changes_id_prefix(self):
        v1 = _v1_doc(_v1_split([_v1_window()]))
        v2 = migration.migrate(v1, editor_name="custom")
        self.assertEqual(v2["root"]["children"][0]["id"], "custom__widget0")


class SizeNormalisationTests(unittest.TestCase):
    """GCD reduction of v1 pixel-derived sibling sizes."""

    def test_uniform_pixel_sizes_collapse_to_default(self):
        v1 = _v1_doc(
            _v1_split(
                [
                    _v1_window(view_direction="axial", size=403),
                    _v1_window(view_direction="sagittal", size=403),
                    _v1_window(view_direction="coronal", size=403),
                ]
            )
        )
        v2 = migration.migrate(v1)
        for child in v2["root"]["children"]:
            self.assertNotIn(
                "size", child,
                f"Uniform sibling sizes should reduce to the schema default; got {child}",
            )

    def test_unequal_sizes_reduce_by_gcd(self):
        v1 = _v1_doc(
            _v1_split(
                [
                    _v1_window(view_direction="axial", size=200),
                    _v1_window(view_direction="sagittal", size=400),
                ]
            )
        )
        v2 = migration.migrate(v1)
        children = v2["root"]["children"]
        # 200:400 reduces to 1:2; the size==1 child drops the field.
        self.assertNotIn("size", children[0])
        self.assertEqual(children[1]["size"], 2)

    def test_coprime_sizes_preserved(self):
        v1 = _v1_doc(
            _v1_split(
                [
                    _v1_window(view_direction="axial", size=403),
                    _v1_window(view_direction="sagittal", size=807),
                ]
            )
        )
        v2 = migration.migrate(v1)
        children = v2["root"]["children"]
        self.assertEqual(children[0]["size"], 403)
        self.assertEqual(children[1]["size"], 807)

    def test_mixed_define_left_alone(self):
        # When some children carry size and others do not, the explicit-vs-
        # default pattern is meaningful in v2 - leave it alone.
        v1 = _v1_doc(
            _v1_split(
                [
                    _v1_window(view_direction="axial", size=300),
                    _v1_window(view_direction="sagittal"),  # no size
                ]
            )
        )
        v2 = migration.migrate(v1)
        children = v2["root"]["children"]
        self.assertEqual(children[0]["size"], 300)
        self.assertNotIn("size", children[1])


class RootWrapTests(unittest.TestCase):
    """Auto-wrap a single-window v1 root into a one-child split."""

    def test_single_window_v1_root_is_wrapped(self):
        v1 = {
            "version": "1.0",
            "isWindow": True,
            "viewDirection": "axial",
            "synchGroup": 1,
            "selectAll": True,
        }
        v2 = migration.migrate(v1)
        self.assertEqual(v2["root"]["type"], "split")
        self.assertEqual(v2["root"]["orientation"], "horizontal")
        self.assertEqual(len(v2["root"]["children"]), 1)
        self.assertEqual(v2["root"]["children"][0]["type"], "window")
        self.assertEqual(v2["root"]["children"][0]["id"], "mxn__widget0")
        # Wrap is a structural fix-up, so no `size` leaks onto the root.
        self.assertNotIn("size", v2["root"])


class EditorNameValidationTests(unittest.TestCase):
    """The `--editor-name` flag's regex matches the C++ engine's constraint."""

    def test_default_name_accepted(self):
        v1 = _v1_doc(_v1_split([_v1_window()]))
        # Should not raise.
        migration.migrate(v1)

    def test_underscore_in_name_rejected(self):
        v1 = _v1_doc(_v1_split([_v1_window()]))
        with self.assertRaises(ValueError):
            migration.migrate(v1, editor_name="bad_name")

    def test_double_underscore_in_name_rejected(self):
        v1 = _v1_doc(_v1_split([_v1_window()]))
        with self.assertRaises(ValueError):
            migration.migrate(v1, editor_name="bad__name")

    def test_leading_digit_rejected(self):
        v1 = _v1_doc(_v1_split([_v1_window()]))
        with self.assertRaises(ValueError):
            migration.migrate(v1, editor_name="2nd")


if __name__ == "__main__":
    unittest.main()
