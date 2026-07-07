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

import numpy as np

import mitk


def test_create_image():
    image = mitk.Image()
    image.initialize("float32", [64, 64, 64])

    assert image.get_dimension() == 3
    assert image.get_dimension(0) == 64

    array = image.as_numpy()
    assert array.shape == (64, 64, 64)
    assert array.dtype == np.float32


def test_image_constructor(tmp_path):
    empty = mitk.Image()
    assert isinstance(empty, mitk.Image)
    assert type(empty) is mitk.Image

    array = np.zeros((2, 3, 4), dtype=np.uint8)
    image = mitk.Image(array, spacing=(1.0, 2.0, 3.0))
    assert isinstance(image, mitk.Image)
    assert image.shape == (2, 3, 4)
    assert image.spacing == (1.0, 2.0, 3.0)

    path = tmp_path / "x.nrrd"
    image.save(str(path))

    loaded_from_str = mitk.Image(str(path))
    loaded_from_path = mitk.Image(Path(path))
    np.testing.assert_array_equal(np.asarray(loaded_from_str), array)
    np.testing.assert_array_equal(np.asarray(loaded_from_path), array)

    assert not hasattr(mitk, "NativeImage")


# ---------------------------------------------------------------------------
# repr / _repr_html_ (Jupyter rich display)
# ---------------------------------------------------------------------------


def _sample_image():
    arr = np.zeros((3, 4, 5), dtype=np.float32)
    return mitk.Image.from_numpy(arr, spacing=(0.5, 1.0, 2.0), origin=(10.0, 20.0, 30.0))


def test_repr_returns_str():
    text = repr(_sample_image())
    assert isinstance(text, str)
    assert text.startswith("Image(")
    assert "(3, 4, 5)" in text        # shape (numpy order)
    assert "float32" in text          # dtype
    assert "(0.5, 1.0, 2.0)" in text  # spacing


def test_repr_uninitialized_does_not_crash():
    text = repr(mitk.Image())
    assert isinstance(text, str)
    assert "uninitialized" in text


def test_repr_html_returns_table():
    html = _sample_image()._repr_html_()
    assert isinstance(html, str)
    assert "<table>" in html


def test_repr_html_contains_geometry():
    html = _sample_image()._repr_html_()
    assert "(3, 4, 5)" in html         # shape
    assert "float32" in html           # dtype
    assert "(0.5, 1.0, 2.0)" in html   # spacing
    assert "origin" in html
    assert "direction" in html


def test_repr_html_uninitialized_does_not_crash():
    html = mitk.Image()._repr_html_()
    assert isinstance(html, str)
