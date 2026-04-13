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
