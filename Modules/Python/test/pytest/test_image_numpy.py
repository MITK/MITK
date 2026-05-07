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


def _load_test_image(path):
    results = mitk.IOUtil.load(str(path))
    assert len(results) >= 1
    return results[0]


def test_image_as_numpy_direct(pic3d_path):
    image = _load_test_image(pic3d_path)

    arr_default = image.as_numpy()
    assert arr_default.flags.writeable
    del arr_default

    arr_ro = image.as_numpy(writeable=False)
    assert not arr_ro.flags.writeable
    del arr_ro

    arr_rw = image.as_numpy(writeable=True)
    assert arr_rw.flags.writeable
    arr_rw[0, 0, 0] = 42
    assert int(arr_rw[0, 0, 0]) == 42


def test_image_as_numpy_accessor(pic3d_path):
    image = _load_test_image(pic3d_path)

    arr_ro = image.as_numpy(use_accessor=True, writeable=False)
    assert not arr_ro.flags.writeable
    del arr_ro

    arr_rw = image.as_numpy(use_accessor=True, writeable=True)
    assert arr_rw.flags.writeable
    arr_rw[0, 0, 0] = 99
    assert int(arr_rw[0, 0, 0]) == 99
    del arr_rw


def test_image_from_numpy():
    array = np.arange(2 * 3 * 4, dtype=np.float32).reshape(4, 3, 2)

    image = mitk.Image.from_numpy(array, spacing=(0.5, 1.0, 2.0), origin=(1.0, 2.0, 3.0))
    assert image.shape == array.shape
    assert image.spacing == (0.5, 1.0, 2.0)
    assert image.origin == (1.0, 2.0, 3.0)
    np.testing.assert_array_equal(image.as_numpy(), array)

    image_from_ctor = mitk.Image(array, spacing=(0.5, 1.0, 2.0))
    assert image_from_ctor.shape == array.shape
    assert image_from_ctor.spacing == (0.5, 1.0, 2.0)


def test_image_array_protocol():
    image = mitk.Image(np.zeros((4, 5, 6), dtype=np.uint8))

    array = np.asarray(image)
    assert array.shape == (4, 5, 6)
    assert array.dtype == np.uint8

    array_float = np.asarray(image, dtype=np.float32)
    assert array_float.dtype == np.float32
