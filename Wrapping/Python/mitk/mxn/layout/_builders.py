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

"""Ergonomic split-returning builders for the common layouts.

Builders return :class:`Split`. Callers wanting a complete document wrap
with ``MxNLayoutDocument.create(root=..., name=...)``. The earlier two-tier
(split + document) is collapsed: convenience cost is one line per script,
and the smaller surface halves the test matrix.

Builders synthesise qualified ids as
``f"{editor_name}{NAMESPACE_DELIMITER}{bare_id_stem}{i}"``. With defaults
(``editor_name='mxn'``, ``bare_id_stem='widget'``, ``start_index=0``) this
yields ``mxn__widget0``, ``mxn__widget1``, ... -- the convention the engine
emits via ``QmitkMxNMultiWidget::SerializeLayout``.
"""

from __future__ import annotations

from collections.abc import Sequence
from itertools import count

from ._model import DEFAULT_GROUP, Group, LayoutWindow, Split
from ._vocabulary import (
    DEFAULT_EDITOR_NAME,
    NAMESPACE_DELIMITER,
    ViewDirection,
)


_GRID_VIEW_CYCLE: tuple[ViewDirection, ...] = (
    ViewDirection.AXIAL,
    ViewDirection.SAGITTAL,
    ViewDirection.CORONAL,
)


def _qualify(editor_name: str, bare_id_stem: str, index: int) -> str:
    return f"{editor_name}{NAMESPACE_DELIMITER}{bare_id_stem}{index}"


def grid(
    rows: int,
    cols: int,
    *,
    view_directions: Sequence[ViewDirection] | None = None,
    group: str | Group = DEFAULT_GROUP,
    editor_name: str = DEFAULT_EDITOR_NAME,
    bare_id_stem: str = "widget",
    start_index: int = 0,
    display_names: Sequence[str | None] | None = None,
) -> Split:
    """Build a ``rows`` x ``cols`` grid of windows.

    Outer split is vertical (rows), inner splits are horizontal (cells in
    each row).

    Args:
        rows: Number of rows (>= 1).
        cols: Number of columns (>= 1).
        view_directions: Per-cell view directions, length ``rows * cols``.
            When omitted, cycles through axial/sagittal/coronal.
        group: Selection group every cell links to. Accepts a group name
            string or a ``Group`` instance.
        editor_name: Editor-name segment for synthesised ids; almost all
            callers leave this at ``'mxn'``.
        bare_id_stem: Prefix of the bare-id segment (the index is
            appended).
        start_index: Starting index for synthesised ids; lets callers
            compose multiple grids without id collisions.
        display_names: Per-cell display labels, length ``rows * cols``.
            Entries that are ``None`` produce no ``name`` field.
    """
    if isinstance(rows, bool) or not isinstance(rows, int) or rows < 1:
        raise ValueError(f"rows must be a positive int, got {rows!r}")
    if isinstance(cols, bool) or not isinstance(cols, int) or cols < 1:
        raise ValueError(f"cols must be a positive int, got {cols!r}")

    cell_count = rows * cols

    if view_directions is None:
        views = [
            _GRID_VIEW_CYCLE[i % len(_GRID_VIEW_CYCLE)]
            for i in range(cell_count)
        ]
    else:
        if len(view_directions) != cell_count:
            raise ValueError(
                f"view_directions has length {len(view_directions)}; "
                f"expected {cell_count} (rows*cols)"
            )
        views = list(view_directions)

    if display_names is None:
        names = [None] * cell_count
    else:
        if len(display_names) != cell_count:
            raise ValueError(
                f"display_names has length {len(display_names)}; "
                f"expected {cell_count} (rows*cols)"
            )
        names = list(display_names)

    selection = group
    counter = count(start_index)
    cells_by_row: list[list[LayoutWindow]] = []
    for r in range(rows):
        cells: list[LayoutWindow] = []
        for c in range(cols):
            cell_idx = r * cols + c
            window_index = next(counter)
            cells.append(
                LayoutWindow.create(
                    id=_qualify(editor_name, bare_id_stem, window_index),
                    view_direction=views[cell_idx],
                    name=names[cell_idx],
                    selection=selection,
                )
            )
        cells_by_row.append(cells)

    # Collapse degenerate splits so the output matches what a human
    # author would write: a 1xN grid is a single horizontal split,
    # an Mx1 grid is a single vertical split, an MxN grid is a vertical
    # of horizontals.
    if rows == 1:
        return Split.horizontal(*cells_by_row[0])
    if cols == 1:
        return Split.vertical(*[row[0] for row in cells_by_row])
    return Split.vertical(*[Split.horizontal(*row) for row in cells_by_row])


def three_up(
    *,
    group: str | Group = DEFAULT_GROUP,
    editor_name: str = DEFAULT_EDITOR_NAME,
    bare_id_stem: str = "widget",
    start_index: int = 0,
) -> Split:
    """Three windows side by side: axial, sagittal, coronal."""
    return grid(
        1,
        3,
        group=group,
        editor_name=editor_name,
        bare_id_stem=bare_id_stem,
        start_index=start_index,
    )


def two_rows_each_direction(
    *,
    group_top: str | Group = DEFAULT_GROUP,
    group_bottom: str | Group = "row2",
    editor_name: str = DEFAULT_EDITOR_NAME,
    bare_id_stem: str = "widget",
    start_index: int = 0,
) -> Split:
    """Two horizontal rows; each row has axial / sagittal / coronal.

    Top row links to ``group_top``, bottom row to ``group_bottom``. By
    default the top is the ``main`` group and the bottom is its own
    ``row2`` group -- matches the in-tree preset
    ``mxnLayout_twoRowsEachDirection.json``.
    """
    counter = count(start_index)

    def row(selection: str | Group) -> Split:
        cells = [
            LayoutWindow.create(
                id=_qualify(editor_name, bare_id_stem, next(counter)),
                view_direction=vd,
                selection=selection,
            )
            for vd in _GRID_VIEW_CYCLE
        ]
        return Split.horizontal(*cells)

    return Split.vertical(row(group_top), row(group_bottom))


def single_window(
    view_direction: ViewDirection | str = ViewDirection.AXIAL,
    *,
    id_qualified: str | None = None,
    editor_name: str = DEFAULT_EDITOR_NAME,
    bare_id_stem: str = "widget",
    start_index: int = 0,
    display_name: str | None = None,
    group: str | Group = DEFAULT_GROUP,
) -> Split:
    """A single-window layout, wrapped in a horizontal split.

    Args:
        view_direction: Anatomical plane.
        id_qualified: Optional explicit qualified id. Must be the canonical
            qualified form (``"mxn__alpha"``); a bare id (``"alpha"``)
            raises ``ValueError`` from the regex check. When ``None``
            (default), synthesises ``f"{editor_name}__{bare_id_stem}{start_index}"``.
        display_name: Optional display label for the cell.
        group: Selection group.
    """
    if id_qualified is None:
        id_qualified = _qualify(editor_name, bare_id_stem, start_index)
    window = LayoutWindow.create(
        id=id_qualified,
        view_direction=view_direction,
        name=display_name,
        selection=group,
    )
    return Split.horizontal(window)
