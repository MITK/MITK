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

import mitk


def test_import():
    assert hasattr(mitk, "__doc__")


def test_autoload_modules():
    modules = mitk.get_loaded_modules()

    assert "MitkCore" in modules
    assert "MitkDICOMImageIO" in modules
    assert "MitkIOExt" in modules
    assert "MitkMultilabelIO" in modules
