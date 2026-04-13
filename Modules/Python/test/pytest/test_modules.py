import mitk


def test_import():
    assert hasattr(mitk, "__doc__")


def test_autoload_modules():
    modules = mitk.get_loaded_modules()

    assert "MitkCore" in modules
    assert "MitkDICOMImageIO" in modules
    assert "MitkIOExt" in modules
    assert "MitkMultilabelIO" in modules
