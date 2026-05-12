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


def test_image_geometry():
    image = mitk.Image()
    image.initialize("float32", [10, 20, 30])

    assert image.shape == (30, 20, 10)
    assert image.ndim == 3
    assert image.dtype == np.float32

    assert image.spacing == (1.0, 1.0, 1.0)
    image.spacing = (0.5, 0.7, 1.5)
    assert image.spacing == (0.5, 0.7, 1.5)

    image.origin = (10.0, 20.0, 30.0)
    assert image.origin == (10.0, 20.0, 30.0)

    np.testing.assert_allclose(image.direction, np.eye(3))
    flipped = np.diag([-1.0, -1.0, 1.0])
    image.direction = flipped
    np.testing.assert_allclose(image.direction, flipped)
    assert image.spacing == (0.5, 0.7, 1.5)

    assert image.time_steps == 1
    assert image.time_geometry is not None
    assert image.time_geometry.count_time_steps() == 1
    assert image.get_geometry(time_step=0) is not None
