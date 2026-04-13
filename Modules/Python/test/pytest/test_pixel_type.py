import mitk


def test_pixel_type():
    pixel_type = mitk.make_pixel_type("float32")
    assert pixel_type is not None
