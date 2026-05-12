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

"""Named preset registry.

Presets are *named, complete documents* registered for lookup. Builders
return :class:`Split`; presets wrap the result with
``MxNLayoutDocument.create(...)`` and are returned ready-to-use.

Named factories (rather than lambdas) so tracebacks point to a real
qualified name when a preset blows up.
"""

from __future__ import annotations

from collections.abc import Callable, Mapping

from ._builders import single_window, three_up, two_rows_each_direction
from ._model import MxNLayoutDocument


def _preset_three_up() -> MxNLayoutDocument:
    return MxNLayoutDocument.create(root=three_up(), name="Three Views")


def _preset_two_rows_each_direction() -> MxNLayoutDocument:
    return MxNLayoutDocument.create(
        root=two_rows_each_direction(),
        name="Two Rows Each Direction",
    )


def _preset_single() -> MxNLayoutDocument:
    return MxNLayoutDocument.create(root=single_window())


PRESETS: Mapping[str, Callable[[], MxNLayoutDocument]] = {
    "three-up": _preset_three_up,
    "two-rows-each-direction": _preset_two_rows_each_direction,
    "single": _preset_single,
}


def preset(name: str) -> MxNLayoutDocument:
    """Return a fresh ``MxNLayoutDocument`` for the named preset.

    Each call invokes the registered factory, so callers receive an
    independent, mutation-safe document.
    """
    try:
        factory = PRESETS[name]
    except KeyError:
        available = ", ".join(sorted(PRESETS.keys()))
        raise KeyError(
            f"unknown preset {name!r}; available presets: [{available}]"
        ) from None
    return factory()


def list_presets() -> list[str]:
    """Names of all registered presets, sorted lexicographically."""
    return sorted(PRESETS.keys())
