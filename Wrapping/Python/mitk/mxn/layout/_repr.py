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

"""Plain-text and HTML representations for the dataclass model.

HTML trust boundary: every string sourced from user-controlled content
(window id, window name, group name, view direction enum value, document
name, link dimension keys) passes through :func:`_html_escape` before
being interpolated. Inline styles only -- no JavaScript, no external
assets, no network.
"""

from __future__ import annotations

from html import escape as _stdlib_html_escape

from ._model import LayoutWindow, MxNLayoutDocument, Split


def _html_escape(text: str) -> str:
    """Escape HTML metacharacters including single and double quotes.

    Centralised so a future audit can grep for ``f"...{...}..."`` in
    this module and confirm every interpolation runs through here.
    """
    return _stdlib_html_escape(text, quote=True)


# --------------------------------------------------------------------------- #
# Plain-text repr
# --------------------------------------------------------------------------- #


def window_repr(window: LayoutWindow) -> str:
    parts = [
        f"id={window.id!r}",
        f"view_direction={window.view_direction.name}",
        f"selection={window.selection!r}",
    ]
    if window.name is not None:
        parts.append(f"name={window.name!r}")
    if window.size != 1:
        parts.append(f"size={window.size}")
    return f"LayoutWindow({', '.join(parts)})"


def split_repr(split: Split) -> str:
    return (
        f"Split(orientation={split.orientation!r}, "
        f"children=<{len(split.children)} nodes>, size={split.size})"
    )


def document_repr(doc: MxNLayoutDocument) -> str:
    label = f"name={doc.name!r}, " if doc.name is not None else ""
    group_count = len(doc.groups)
    window_count = sum(1 for _ in doc.windows())
    return (
        f"MxNLayoutDocument({label}version={doc.version!r}, "
        f"windows={window_count}, groups={group_count})"
    )


# --------------------------------------------------------------------------- #
# HTML repr (notebook)
# --------------------------------------------------------------------------- #


_WINDOW_STYLE = (
    "border:1px solid #999; padding:6px 8px; margin:2px; "
    "font-family:sans-serif; min-width:120px; min-height:60px; "
    "display:inline-block; vertical-align:top; background:#fafafa;"
)
_TABLE_STYLE = (
    "border-collapse:collapse; margin:2px;"
)
_CELL_STYLE = (
    "padding:0; vertical-align:top;"
)
_DOC_HEADER_STYLE = (
    "font-family:sans-serif; padding:4px 0; color:#333;"
)


def window_html(window: LayoutWindow) -> str:
    """Render a single window as a bordered ``<div>``."""
    parts: list[str] = []
    parts.append(f'<div style="{_WINDOW_STYLE}">')
    if window.name is not None:
        parts.append(
            f'<div style="font-weight:600; margin-bottom:2px;">'
            f"{_html_escape(window.name)}</div>"
        )
    parts.append(
        f'<div style="font-family:monospace; font-size:0.85em; color:#666;">'
        f"{_html_escape(window.id)}</div>"
    )
    parts.append(
        f'<div style="font-size:0.9em;">view: '
        f"{_html_escape(window.view_direction.value)}</div>"
    )
    # Selection link first (it is the only required v2 dimension); other
    # link dimensions follow in sorted order so the output is stable.
    selection = window.selection
    parts.append(
        f'<div style="font-size:0.9em;">selection: '
        f"{_html_escape(selection)}</div>"
    )
    extras = [
        (dim, value if isinstance(value, str) else value.group)
        for dim, value in window._links
        if dim != "selection"
    ]
    for dim, value in extras:
        parts.append(
            f'<div style="font-size:0.9em;">'
            f"{_html_escape(dim)}: {_html_escape(value)}</div>"
        )
    parts.append("</div>")
    return "".join(parts)


def _node_html(node: Split | LayoutWindow) -> str:
    if isinstance(node, LayoutWindow):
        return window_html(node)
    return split_html(node)


def split_html(split: Split) -> str:
    """Render a split as a tiny ``<table>`` whose layout (rows for
    vertical, single row for horizontal) mirrors the splitter orientation.
    """
    children_html = [_node_html(c) for c in split.children]
    if split.orientation == "horizontal":
        cells = "".join(f'<td style="{_CELL_STYLE}">{html}</td>' for html in children_html)
        return f'<table style="{_TABLE_STYLE}"><tr>{cells}</tr></table>'
    rows = "".join(
        f'<tr><td style="{_CELL_STYLE}">{html}</td></tr>' for html in children_html
    )
    return f'<table style="{_TABLE_STYLE}">{rows}</table>'


def document_html(doc: MxNLayoutDocument) -> str:
    parts: list[str] = []
    if doc.name is not None:
        parts.append(
            f'<div style="{_DOC_HEADER_STYLE} font-weight:600;">'
            f"{_html_escape(doc.name)}</div>"
        )
    group_summary = ", ".join(
        f"{_html_escape(name)} ({'select-all' if g.select_all else 'curated'})"
        for name, g in sorted(doc.groups.items())
    )
    parts.append(
        f'<div style="{_DOC_HEADER_STYLE} font-size:0.9em; color:#555;">'
        f"version {_html_escape(doc.version)} &middot; groups: "
        f"{group_summary or '(none)'}"
        f"</div>"
    )
    parts.append(split_html(doc.root))
    return "".join(parts)
