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

"""Tests for the public surface of `mitk.mxn.layout`.

The vocabulary module is private; every name it defines for users reaches
them through the package.
"""

from __future__ import annotations

import enum

import mitk.mxn.layout as layout
from mitk.mxn.layout import _vocabulary as vocabulary


def _vocabulary_names() -> set[str]:
    return {
        name
        for name, value in vars(vocabulary).items()
        if not name.startswith("_")
        and (
            name.isupper()
            or (
                isinstance(value, type)
                and issubclass(value, enum.Enum)
                and value.__module__ == vocabulary.__name__
            )
        )
    }


def test_every_vocabulary_name_is_reexported() -> None:
    names = _vocabulary_names()
    assert names - set(layout.__all__) == set()
    for name in names:
        assert getattr(layout, name) is getattr(vocabulary, name)
        assert name in layout.__doc__
