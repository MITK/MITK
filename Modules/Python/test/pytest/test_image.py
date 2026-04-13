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
