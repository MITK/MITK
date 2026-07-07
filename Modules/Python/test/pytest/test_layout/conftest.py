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

"""Fixtures for the `mitk.mxn.layout` test suite.

The fixtures resolve the schema and in-tree preset paths from the source
tree by walking up from this file. They fail soft when running in a
wheel-test environment where the source tree is not on disk: the fixture
returns ``None`` and the depending test should skip itself with a clear
reason.

Source-tree CI runs cover the schema integration; wheel-test CI runs
cover everything else. Snapshotting the schema into ``Wrapping/Python``
would invite drift, so we keep the test honest at the cost of a skip in
the wheel environment.
"""

from __future__ import annotations

from pathlib import Path

import pytest


def _find_repo_root() -> Path | None:
    """Walk up from `__file__` looking for the marker that identifies the
    MITK source tree. Returns None when running outside it.
    """
    here = Path(__file__).resolve()
    for ancestor in (here, *here.parents):
        if (ancestor / "Modules" / "QtWidgets" / "resource").is_dir():
            return ancestor
    return None


@pytest.fixture(scope="session")
def schema_path() -> Path | None:
    """Path to `mxn-layout-v2.schema.json`, or None when unavailable."""
    root = _find_repo_root()
    if root is None:
        return None
    candidate = root / "Modules" / "QtWidgets" / "resource" / "mxn-layout-v2.schema.json"
    return candidate if candidate.is_file() else None


@pytest.fixture(scope="session")
def two_rows_preset_path() -> Path | None:
    """Path to the in-tree preset emitted by the C++ engine, or None."""
    root = _find_repo_root()
    if root is None:
        return None
    candidate = (
        root / "Modules" / "QtWidgets" / "resource" / "mxnLayout_twoRowsEachDirection.json"
    )
    return candidate if candidate.is_file() else None


@pytest.fixture
def fresh_doc():
    """Fresh ``MxNLayoutDocument`` with a small, multi-group fixture.

    Importing the DSL inside the fixture (rather than at module top) keeps
    each test file independently runnable when the DSL is present and lets
    `pytest --collect-only` enumerate the suite even when it is not.
    """
    from mitk.mxn.layout import (
        Group,
        LayoutWindow,
        MxNLayoutDocument,
        Split,
        ViewDirection,
    )

    return MxNLayoutDocument.create(
        root=Split.vertical(
            Split.horizontal(
                LayoutWindow.create(
                    id="mxn__widget0",
                    view_direction=ViewDirection.AXIAL,
                    name="Top - Axial",
                ),
                LayoutWindow.create(
                    id="mxn__widget1",
                    view_direction=ViewDirection.SAGITTAL,
                    name="Top - Sagittal",
                ),
            ),
            Split.horizontal(
                LayoutWindow.create(
                    id="mxn__widget2",
                    view_direction=ViewDirection.CORONAL,
                    selection="row2",
                ),
                LayoutWindow.create(
                    id="mxn__widget3",
                    view_direction=ViewDirection.AXIAL,
                    selection="row2",
                ),
            ),
        ),
        name="Mixed Fixture",
        groups={
            "main": Group("main", select_all=True),
            "row2": Group("row2", select_all=False),
        },
    )
