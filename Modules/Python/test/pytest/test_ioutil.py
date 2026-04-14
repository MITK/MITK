import os
import pytest
import numpy as np

import mitk


def test_ioutil_reader_preferences(tmp_path):
    array = np.arange(60, dtype=np.float32).reshape(3, 4, 5)
    image = mitk.Image(array, spacing=(0.5, 0.7, 1.1))
    path = tmp_path / "prefs.nrrd"

    image.save(str(path))

    def assert_loads(**kwargs):
        results = mitk.IOUtil.load(str(path), **kwargs)
        assert len(results) >= 1
        np.testing.assert_array_equal(np.asarray(results[0]), array)

    assert_loads(reader_preferences=None, reader_blacklist=None)
    assert_loads(reader_preferences=[], reader_blacklist=[])
    assert_loads(reader_preferences=["ITK NrrdImageIO"])
    assert_loads(reader_preferences=["NonExistentReader"])
    assert_loads(reader_blacklist=["NonExistentReader"])
    assert_loads(
        reader_preferences=["ITK NrrdImageIO"],
        reader_blacklist=["NonExistentReader"],
    )
    assert_loads(
        reader_preferences=("ITK NrrdImageIO",),
        reader_blacklist=("NonExistentReader",),
    )

# ---------------------------------------------------------------------------
# load(): returns correct Python types
# ---------------------------------------------------------------------------

class TestIOUtilLoad:

    def test_load_image_returns_image(self, mitk_data_dir):
        path = os.path.join(mitk_data_dir, "Pic3D.nrrd")
        results = mitk.IOUtil.load(path)
        assert len(results) >= 1
        assert isinstance(results[0], mitk.Image)

    def test_load_segmentation_returns_multilabelsegmentation(self, mitk_data_dir):
        path = os.path.join(mitk_data_dir, "Multilabel", "MultilabelSegmentation.nrrd")
        results = mitk.IOUtil.load(path)
        assert len(results) >= 1
        assert isinstance(results[0], mitk.MultiLabelSegmentation)


# ---------------------------------------------------------------------------
# save(): accepts Image and MultiLabelSegmentation, rejects other types
# ---------------------------------------------------------------------------

class TestIOUtilSave:

    def test_save_image(self, tmp_path):
        arr = np.zeros((4, 8, 8), dtype=np.uint16)
        img = mitk.Image.from_numpy(arr, spacing=(1.0, 1.0, 1.0))
        path = str(tmp_path / "image.nrrd")
        mitk.IOUtil.save(img, path)
        assert os.path.exists(path)

    def test_save_segmentation(self, tmp_path):
        arr = np.zeros((4, 8, 8), dtype=np.uint16)
        ref = mitk.Image.from_numpy(arr, spacing=(1.0, 1.0, 1.0))
        seg = mitk.MultiLabelSegmentation(ref)
        seg.add_label("Liver", (1.0, 0.0, 0.0), 0)
        path = str(tmp_path / "seg.nrrd")
        mitk.IOUtil.save(seg, path)
        assert os.path.exists(path)

    def test_save_wrong_type_raises(self, tmp_path):
        with pytest.raises(TypeError):
            mitk.IOUtil.save(object(), str(tmp_path / "bad.nrrd"))
    