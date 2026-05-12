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
GROUP_NAME_PATTERN: Final[str] = r"^[A-Za-z0-9_.-]+$"


class LinkDimension(str, Enum):
    """Synchronization dimension keys recognised by the v2.0 schema.

    v2.0 knows only SELECTION. v3.0 will additively introduce ZOOM, TIME,
    CROSSHAIR, etc. APIs that take a dimension accept ``LinkDimension`` or a
    plain ``str`` so callers can pass forward-compat strings before the enum
    is updated.
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
