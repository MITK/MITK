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

"""Vocabulary for the `mitk.mxn.layout` DSL.

Enums, constants, and regex patterns mirroring the v2.0 schema
(`Modules/QtWidgets/resource/mxn-layout-v2.schema.json`). The whole package
imports from here so the schema-derived strings live in exactly one place.
"""

from __future__ import annotations

from enum import Enum
from typing import Final


LAYOUT_VERSION: Final[str] = "2.0"
DEFAULT_GROUP_NAME: Final[str] = "main"
DEFAULT_EDITOR_NAME: Final[str] = "mxn"
NAMESPACE_DELIMITER: Final[str] = "__"

# Mirror of the schema's `window.id` pattern (qualified form).
WINDOW_ID_PATTERN: Final[str] = r"^[A-Za-z][A-Za-z0-9.-]*__[A-Za-z0-9_.-]+$"

# Mirror of the schema's `groups.propertyNames.pattern` and the `links.selection.pattern`.
# Necessary but not sufficient: the schema also excludes RESERVED_GROUP_ID.
GROUP_NAME_PATTERN: Final[str] = r"^[A-Za-z0-9_.-]+$"

# Mirror of the schema's `not` clause on group names: the engine reserves this
# id, in every spelling, for the group that follows the application's value.
RESERVED_GROUP_ID: Final[str] = "global"


class LinkDimension(str, Enum):
    """Synchronization dimension keys recognised by the v2.0 schema.

    The DSL writes 2.0, which knows only SELECTION. 3.0
    (`mxn-layout-v3.schema.json`) adds pan, zoom, slice, crosshair,
    orientation, windowing and lut, and requires `time`, which accepts only
    the reserved group `global`; the DSL writes none of them. APIs that take
    a dimension accept ``LinkDimension`` or a plain ``str``.
    """

    SELECTION = "selection"


class ViewDirection(str, Enum):
    """Anatomical plane shown in a render window. Lowercase string values
    matching the schema's `view_direction` enum and the rest of the
    rendering REST surface.
    """

    AXIAL = "axial"
    SAGITTAL = "sagittal"
    CORONAL = "coronal"
    ORIGINAL = "original"
