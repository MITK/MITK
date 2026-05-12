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

"""Getting started with `mitk.mxn.layout`.

A runnable tour of the DSL: build, inspect, transform, round-trip,
validate. Mirrored by ``01_getting_started.ipynb`` -- keep both in sync
when editing.

Smoke-tested by ``test_examples_smoke.py`` in the pytest suite that
``mitkPythonBindingsTest`` drives; any regression in a covered surface
causes CI to fail.
"""

from __future__ import annotations

import json
import tempfile
from pathlib import Path

from mitk.mxn.layout import (
    LayoutWindow,
    MxNLayoutDocument,
    Split,
    ViewDirection,
    grid,
    load_preset,
    save_preset,
)


def main() -> None:
    print("=" * 70)
    print("1) Build a 2 x 3 grid with display labels")
    print("=" * 70)
    doc = MxNLayoutDocument.create(
        root=grid(
            2,
            3,
            display_names=[
                "Tumor - Axial",
                "Tumor - Sagittal",
                "Tumor - Coronal",
                None,
                None,
                None,
            ],
        ),
        name="Tumor MPR + Reference",
    )
    print(doc)
    print("ids in pre-order:", doc.window_ids())

    print()
    print("=" * 70)
    print("2) Compose a custom layout, then renumber ids cleanly")
    print("=" * 70)
    custom = Split.vertical(
        Split.horizontal(
            LayoutWindow.create(id="mxn__handauthored.alpha", view_direction="axial"),
            LayoutWindow.create(id="mxn__handauthored.beta", view_direction="sagittal"),
        ),
        LayoutWindow.create(id="mxn__handauthored.gamma", view_direction="coronal"),
    )
    custom_doc = MxNLayoutDocument.create(root=custom, name="Hand-stitched")
    print("hand-authored ids:", custom_doc.window_ids())
    renumbered = custom_doc.with_default_ids()
    print("after with_default_ids():", renumbered.window_ids())

    print()
    print("=" * 70)
    print("3) Save and load a preset round-trip via a temp file")
    print("=" * 70)
    with tempfile.TemporaryDirectory() as tmpdir:
        path = Path(tmpdir) / "preset.json"
        save_preset(path, doc)
        round_tripped = load_preset(path)
        assert round_tripped == doc, "round-trip should preserve equality"
        print("round-trip equal:", round_tripped == doc)
        print("file (first 240 chars):")
        print(path.read_text(encoding="utf-8")[:240], "...")

    print()
    print("=" * 70)
    print("4) Rename display labels with the selector")
    print("=" * 70)
    relabelled = doc.select_windows.where(view_direction=ViewDirection.AXIAL).with_display_name(
        lambda w: f"Axial ({w.id.split('__', 1)[1]})"
    )
    for w in relabelled.windows():
        print(f"  {w.id}: {w.view_direction.value} -> name={w.name!r}")

    print()
    print("=" * 70)
    print("5) Re-group windows via the selector + auto-materialise the group")
    print("=" * 70)
    regrouped = doc.select_windows.where(view_direction="axial").link_to("row2")
    print("groups after link_to('row2'):", sorted(regrouped.groups.keys()))
    print(
        "row2 members:",
        [w.id for w in regrouped.select_windows.where(group="row2")],
    )

    print()
    print("=" * 70)
    print("6) Validation rejects malformed input at the cell boundary")
    print("=" * 70)
    try:
        LayoutWindow.create(id="widget0", view_direction="axial")
    except ValueError as exc:
        print("rejected as expected:", exc)
    else:
        raise AssertionError("expected bare-id LayoutWindow.create to raise")

    print()
    print("=" * 70)
    print("7) Inspect the final document as JSON")
    print("=" * 70)
    print(json.dumps(doc.to_json(), indent=2)[:400], "...")

    print()
    print("Done.")


if __name__ == "__main__":
    main()
