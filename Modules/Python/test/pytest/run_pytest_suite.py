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

from pathlib import Path
import os

os.environ.setdefault("PYTEST_DISABLE_PLUGIN_AUTOLOAD", "1")

import pytest


suite_dir = Path(__file__).resolve().parent
exit_code = pytest.main(["--tb=short", "-ra", f"--mitk-data-dir={_mitk_data_dir}", str(suite_dir)])

if exit_code != 0:
    raise RuntimeError(f"pytest failed with exit code {exit_code}")
