import os
import pytest


@pytest.fixture
def mitk_data_dir():
    """Return the MITK test-data directory.

    The path is injected by CTest via the MITK_DATA_DIR environment variable
    (set from the CMake MITK_DATA_DIR variable that the superbuild populates).
    Tests that depend on this fixture are automatically skipped when the
    variable is not present (e.g. local runs without a superbuild).
    """
    d = os.environ.get("MITK_DATA_DIR", "")
    if not d:
        pytest.skip("MITK_DATA_DIR not set")
    return d
