"""Smoke tests for the MITK wheel.

Run in a clean virtual environment after pip install:

  python -m venv test_env
  test_env/Scripts/activate  (or source test_env/bin/activate)
  pip install dist/mitk-*.whl
  python Wrapping/Python/wheel/test_wheel.py
"""

import sys


def test_import():
    import mitk
    assert hasattr(mitk, "__version__")
    print(f"  mitk {mitk.__version__} imported successfully")


def test_create_image():
    import mitk
    import numpy as np

    img = mitk.Image.new()
    img.initialize("float32", [64, 64, 64])

    assert img.get_dimension() == 3
    assert img.get_dimension(0) == 64

    arr = img.as_numpy()
    assert arr.shape == (64, 64, 64)
    assert arr.dtype == np.float32
    print("  Image creation and as_numpy() OK")


def test_numpy_roundtrip():
    import mitk
    import numpy as np

    img = mitk.Image.new()
    img.initialize("uint8", [10, 10])

    # Write via writable accessor, then release it before reading
    arr = img.as_numpy(writeable=True)
    arr[5, 5] = 42
    del arr  # release write accessor

    arr2 = img.as_numpy()
    assert arr2[5, 5] == 42
    print("  NumPy roundtrip OK")


def test_point_vector_types():
    import mitk

    p2 = mitk.Point2D(1.0, 2.0)
    assert p2.x == 1.0 and p2.y == 2.0

    p3 = mitk.Point3D(1.0, 2.0, 3.0)
    assert p3.z == 3.0

    v2 = mitk.Vector2D(4.0, 5.0)
    assert v2.x == 4.0

    v3 = mitk.Vector3D(4.0, 5.0, 6.0)
    assert v3.z == 6.0
    print("  Point and Vector types OK")


def test_pixel_type():
    import mitk

    pt = mitk.make_pixel_type("float32")
    assert pt is not None
    print("  PixelType OK")


def test_autoload_modules():
    import mitk

    modules = mitk.get_autoloaded_modules()

    # MitkCore must always be loaded (it's a direct dependency of the bindings)
    assert "MitkCore" in modules, f"MitkCore not loaded. Loaded: {modules}"

    # Auto-load IO modules should be loaded via CppMicroServices auto-loading.
    # These are installed into MitkCore/ subdirectory in the wheel.
    expected_autoloads = [
        "MitkDICOMImageIO",
        "MitkIOExt",
        "MitkMultilabelIO",
    ]

    missing = [m for m in expected_autoloads if m not in modules]
    assert not missing, f"Auto-load modules not loaded: {missing}. Loaded: {modules}"

    print(f"  Auto-load OK ({len(modules)} modules loaded: {', '.join(sorted(modules))})")


def main():
    tests = [
        test_import,
        test_create_image,
        test_numpy_roundtrip,
        test_point_vector_types,
        test_pixel_type,
        test_autoload_modules,
    ]

    passed = 0
    failed = 0

    for test in tests:
        name = test.__name__
        try:
            test()
            passed += 1
        except Exception as e:
            print(f"  FAILED: {name}: {e}", file=sys.stderr)
            failed += 1

    print(f"\n{passed} passed, {failed} failed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
