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

"""``mitk.mxn.layout`` -- typed Python DSL for MxN multi-widget layouts.

A self-contained, transport-free Python DSL for constructing, inspecting,
validating, and round-tripping MxN multi-widget layout documents (schema
v2.0). Round-trips byte-stable with the wire/file JSON consumed by
``QmitkMxNMultiWidget::ApplyLayout`` and produced by
``QmitkMxNMultiWidget::SerializeLayout``.

Window identity vs. display label
---------------------------------
Every window has a required ``id`` (qualified form ``<editor_name>__<bare_id>``,
e.g. ``mxn__widget0``) and an optional ``name`` (free-form display label).
Routing -- REST URL paths, engine-side render-window names, persisted state
keys -- uses ``id`` exclusively. The DSL never adds, strips, or otherwise
translates a prefix; the qualified form is canonical everywhere.

Public surface
--------------
- :class:`MxNLayoutDocument`, :class:`Split`, :class:`LayoutWindow`,
  :class:`Group`, :class:`Link` -- typed dataclasses.
- :func:`grid`, :func:`three_up`, :func:`two_rows_each_direction`,
  :func:`single_window` -- split-returning builders.
- :data:`PRESETS`, :func:`preset`, :func:`list_presets` -- named registry.
- :func:`load_preset`, :func:`save_preset` -- file I/O.
- :class:`MxNWindowSelector` -- pandas-style filter + bulk transform.
- Vocabulary: :class:`LinkDimension`, :class:`ViewDirection`,
  :data:`LAYOUT_VERSION`, :data:`DEFAULT_GROUP_NAME`,
  :data:`DEFAULT_GROUP`, :data:`DEFAULT_EDITOR_NAME`,
  :data:`NAMESPACE_DELIMITER`, :data:`WINDOW_ID_PATTERN`,
  :data:`GROUP_NAME_PATTERN`.
"""

from __future__ import annotations

from ._builders import grid, single_window, three_up, two_rows_each_direction
from ._io import load_preset, save_preset
from ._model import (
    DEFAULT_GROUP,
    Group,
    LayoutWindow,
    Link,
    LinkLike,
    LinkValue,
    MxNLayoutDocument,
    Split,
)
from ._presets import PRESETS, list_presets, preset
from ._selector import MxNWindowSelector
from ._vocabulary import (
    DEFAULT_EDITOR_NAME,
    DEFAULT_GROUP_NAME,
    GROUP_NAME_PATTERN,
    LAYOUT_VERSION,
    NAMESPACE_DELIMITER,
    WINDOW_ID_PATTERN,
    LinkDimension,
    ViewDirection,
)


__all__ = [
    # Vocabulary
    "LAYOUT_VERSION",
    "DEFAULT_GROUP_NAME",
    "DEFAULT_GROUP",
    "DEFAULT_EDITOR_NAME",
    "NAMESPACE_DELIMITER",
    "WINDOW_ID_PATTERN",
    "GROUP_NAME_PATTERN",
    "LinkDimension",
    "ViewDirection",
    # Model
    "Group",
    "Link",
    "LinkValue",
    "LinkLike",
    "LayoutWindow",
    "Split",
    "MxNLayoutDocument",
    # Selector
    "MxNWindowSelector",
    # Builders
    "grid",
    "three_up",
    "two_rows_each_direction",
    "single_window",
    # Presets
    "PRESETS",
    "preset",
    "list_presets",
    # File I/O
    "load_preset",
    "save_preset",
]
