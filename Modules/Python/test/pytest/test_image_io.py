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

import numpy as np

import mitk


def test_image_load_save_roundtrip(tmp_path):
    array = np.arange(60, dtype=np.float32).reshape(3, 4, 5)
    image = mitk.Image(array, spacing=(0.5, 0.7, 1.1))
    path = tmp_path / "out.nrrd"

    image.save(str(path))

    loaded = mitk.Image.load(str(path))
    np.testing.assert_array_equal(np.asarray(loaded), array)
    assert loaded.spacing == (0.5, 0.7, 1.1)

    loaded_from_str = mitk.Image(str(path))
    assert loaded_from_str.spacing == (0.5, 0.7, 1.1)

    from pathlib import Path
    loaded_from_path = mitk.Image(Path(path))
    assert loaded_from_path.spacing == (0.5, 0.7, 1.1)

    results = mitk.IOUtil.load(str(path))
    assert len(results) >= 1
    np.testing.assert_array_equal(np.asarray(results[0]), array)
