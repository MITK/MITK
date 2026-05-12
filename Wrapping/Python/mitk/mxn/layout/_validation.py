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

"""Per-field validation rule helpers.

These helpers operate on primitive values; they have no knowledge of the
dataclass model. Cross-document rules (id uniqueness, group resolution)
live on `MxNLayoutDocument.validate()` since they need access to the
whole tree.

Single source of truth: per-window structural rules are enforced once in
`LayoutWindow.create`; cross-document rules are enforced in
`MxNLayoutDocument.validate()`.
"""

from __future__ import annotations

import re
from typing import Final

from ._vocabulary import (
    GROUP_NAME_PATTERN,
    LAYOUT_VERSION,
    LinkDimension,
    ViewDirection,
    WINDOW_ID_PATTERN,
)


_WINDOW_ID_RE: Final = re.compile(WINDOW_ID_PATTERN)
_GROUP_NAME_RE: Final = re.compile(GROUP_NAME_PATTERN)
_VIEW_DIRECTION_VALUES: Final = frozenset(v.value for v in ViewDirection)


def validate_window_id(value: str) -> None:
    """Reject anything that does not match the qualified-id pattern.

    Pre-condition: caller passes a `str`.
    """
    if not isinstance(value, str):
        raise ValueError(f"window id must be a string, got {type(value).__name__}")
    if not _WINDOW_ID_RE.fullmatch(value):
        raise ValueError(
            f"window id {value!r} does not match qualified-id pattern "
            f"{WINDOW_ID_PATTERN!r}; expected '<editor_name>__<bare_id>'"
        )


def validate_window_name(value: str | None) -> None:
    """Display name: optional; when set, must be a non-empty string."""
    if value is None:
        return
    if not isinstance(value, str):
        raise ValueError(f"window name must be a string or None, got {type(value).__name__}")
    if len(value) < 1:
        raise ValueError("window name must be a non-empty string when set; pass None to omit")


def validate_group_name(value: str) -> None:
    """Group name: must match the URL-segment-safe pattern."""
    if not isinstance(value, str):
        raise ValueError(f"group name must be a string, got {type(value).__name__}")
    if not _GROUP_NAME_RE.fullmatch(value):
        raise ValueError(
            f"group name {value!r} does not match pattern {GROUP_NAME_PATTERN!r}"
        )


def validate_view_direction(value: object) -> None:
    """Accept either a `ViewDirection` enum member or its string value."""
    if isinstance(value, ViewDirection):
        return
    if isinstance(value, str) and value in _VIEW_DIRECTION_VALUES:
        return
    raise ValueError(
        f"view_direction must be one of {sorted(_VIEW_DIRECTION_VALUES)}, "
        f"got {value!r}"
    )


def validate_size(value: int) -> None:
    """Splitter weight: positive integer; bool is rejected."""
    # bool is a subclass of int -- rule that out explicitly so True/False
    # don't masquerade as a valid weight.
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError(f"size must be an int, got {type(value).__name__}")
    if value < 1:
        raise ValueError(f"size must be >= 1, got {value}")


def validate_dimension_key(value: object) -> str:
    """Coerce a dimension key to its string form; reject non-string keys.

    Free-form strings are accepted (forward-compat for v3 dimensions); the
    caller cross-checks the v2-required `selection` key separately.
    """
    if isinstance(value, LinkDimension):
        return value.value
    if isinstance(value, str):
        if len(value) < 1:
            raise ValueError("link dimension key must be a non-empty string")
        return value
    raise ValueError(
        f"link dimension key must be LinkDimension or str, got {type(value).__name__}"
    )


def validate_layout_version(value: str) -> None:
    """The DSL targets a single schema version; anything else is rejected."""
    if value != LAYOUT_VERSION:
        raise ValueError(
            f"DSL targets version {LAYOUT_VERSION!r}; document is version {value!r}"
        )
