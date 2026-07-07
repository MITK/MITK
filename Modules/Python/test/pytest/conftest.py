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

import pytest


def pytest_addoption(parser):
    parser.addoption("--mitk-data-dir", default="", help="Path to the MITK test data directory")


@pytest.fixture(scope="session")
def data_dir(request):
    path = request.config.getoption("--mitk-data-dir")
    assert path, "--mitk-data-dir is not set"

    data_path = Path(path)
    assert data_path.is_dir(), f"MITK test data directory does not exist: {data_path}"
    return data_path


@pytest.fixture(scope="session")
def pic3d_path(data_dir):
    path = data_dir / "Pic3D.nrrd"
    assert path.is_file(), f"Expected test image is missing: {path}"
    return path
