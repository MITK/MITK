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

import gzip

import numpy as np
import pytest

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


def _make_4d_array():
    # Every voxel holds its own position, so a swapped axis changes the values.
    t, z, y, x = np.indices((5, 3, 4, 6))
    return (t * 1000 + z * 100 + y * 10 + x).astype(np.int32)


def _read_nrrd_int32(path):
    raw = path.read_bytes()
    header_end = raw.index(b"\n\n") + 2
    header = raw[:header_end].decode("ascii")
    fields = dict(line.split(": ", 1) for line in header.splitlines() if ": " in line)

    assert fields["type"] == "int"
    assert fields["endian"] == "little"
    assert fields["encoding"] in ("raw", "gzip")

    payload = raw[header_end:]

    if fields["encoding"] == "gzip":
        payload = gzip.decompress(payload)

    # NRRD lists the fastest axis first.
    sizes = [int(size) for size in fields["sizes"].split()]
    return np.frombuffer(payload, dtype="<i4").reshape(sizes[::-1])


def test_image_4d_whole_views():
    array = _make_4d_array()
    image = mitk.Image.from_numpy(array)

    np.testing.assert_array_equal(image.as_numpy(), array)
    np.testing.assert_array_equal(image.array, array)
    np.testing.assert_array_equal(np.asarray(image), array)

    accessor_view = image.as_numpy(use_accessor=True, writeable=False)
    np.testing.assert_array_equal(accessor_view, array)


def test_image_4d_time_step_views():
    array = _make_4d_array()
    image = mitk.Image.from_numpy(array)

    for t in range(array.shape[0]):
        np.testing.assert_array_equal(image.as_numpy(time_step=t), array[t])

        accessor_view = image.as_numpy(use_accessor=True, writeable=False, time_step=t)
        np.testing.assert_array_equal(accessor_view, array[t])
        del accessor_view

    image.as_numpy(time_step=2)[0, 0, 0] = -1
    assert image.as_numpy()[2, 0, 0, 0] == -1

    with pytest.raises(mitk.Exception):
        image.as_numpy(time_step=array.shape[0])


def test_image_4d_filled_per_time_step():
    array = _make_4d_array()
    image = mitk.Image()
    image.initialize("int32", list(reversed(array.shape)))

    for t in range(array.shape[0]):
        image.as_numpy(time_step=t)[...] = array[t]

    np.testing.assert_array_equal(np.asarray(image), array)


def test_image_4d_layout_matches_nrrd(tmp_path):
    array = _make_4d_array()
    path = tmp_path / "image4d.nrrd"
    mitk.Image.from_numpy(np.asfortranarray(array)).save(str(path))

    np.testing.assert_array_equal(_read_nrrd_int32(path), array)

    loaded = mitk.Image.load(str(path))
    np.testing.assert_array_equal(loaded.as_numpy(), array)

    for t in range(array.shape[0]):
        np.testing.assert_array_equal(loaded.as_numpy(time_step=t), array[t])
