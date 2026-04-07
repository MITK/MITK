"""Smoke tests for the MITK wheel.

Usage:
  python test_wheel.py --build-dir <MITK-build>     # discover wheel in build dir
  python test_wheel.py                               # discover wheel in CWD

The script creates a temporary virtual environment, installs the wheel,
runs the tests, and cleans up automatically.
"""

import argparse
import glob
import os
import subprocess
import sys
import tempfile


# ---------------------------------------------------------------------------
# Test functions (run inside the clean venv via --run-tests)
# ---------------------------------------------------------------------------

def test_import():
    import mitk
    assert hasattr(mitk, "__version__")
    print(f"  mitk {mitk.__version__} imported successfully")


def test_create_image():
    import mitk
    import numpy as np

    img = mitk.NativeImage.new()
    img.initialize("float32", [64, 64, 64])

    assert img.get_dimension() == 3
    assert img.get_dimension(0) == 64

    arr = img.as_numpy()
    assert arr.shape == (64, 64, 64)
    assert arr.dtype == np.float32
    print("  Image creation and as_numpy() OK")


def test_numpy_roundtrip_direct():
    """Default direct (unlocked) path: writes to one numpy view are
    immediately visible to a second view, no del needed."""
    import mitk
    import numpy as np

    img = mitk.NativeImage.new()
    img.initialize("uint8", [10, 10])

    arr = img.as_numpy(writeable=True)
    arr[5, 5] = 42

    # No del required: direct path holds no accessor lock.
    arr2 = img.as_numpy()
    assert arr2[5, 5] == 42
    print("  NumPy roundtrip (direct path) OK")


def test_numpy_roundtrip_accessor():
    """Opt-in accessor path: legacy locked semantics still work."""
    import mitk
    import numpy as np

    img = mitk.NativeImage.new()
    img.initialize("uint8", [10, 10])

    arr = img.as_numpy(use_accessor=True, writeable=True)
    arr[3, 3] = 17
    del arr  # release write accessor

    arr2 = img.as_numpy(use_accessor=True)
    assert arr2[3, 3] == 17
    del arr2
    print("  NumPy roundtrip (accessor path) OK")


def test_image_geometry():
    """WP-1: spacing/origin/direction read-write, shape/ndim/dtype, time geometry."""
    import mitk
    import numpy as np

    img = mitk.NativeImage.new()
    img.initialize("float32", [10, 20, 30])

    # numpy convention: shape is reversed wrt MITK dim order
    assert img.shape == (30, 20, 10)
    assert img.ndim == 3
    assert img.dtype == np.float32

    # default spacing is 1.0 in every direction
    assert img.spacing == (1.0, 1.0, 1.0)
    img.spacing = (0.5, 0.7, 1.5)
    assert img.spacing == (0.5, 0.7, 1.5)

    # origin
    img.origin = (10.0, 20.0, 30.0)
    assert img.origin == (10.0, 20.0, 30.0)

    # direction (default identity); set to a flipped LPS variant
    np.testing.assert_array_almost_equal(img.direction, np.eye(3))
    flipped = np.diag([-1.0, -1.0, 1.0])
    img.direction = flipped
    np.testing.assert_array_almost_equal(img.direction, flipped)
    # spacing must be preserved across the direction set
    assert img.spacing == (0.5, 0.7, 1.5)

    # time geometry
    assert img.time_steps == 1
    assert img.time_geometry is not None
    assert img.time_geometry.count_time_steps() == 1
    assert img.get_geometry(time_step=0) is not None
    print("  Image geometry (WP-1) OK")


def test_image_from_numpy():
    """WP-2: factory from a numpy array, with geometry overrides."""
    import mitk
    import numpy as np

    arr = np.arange(2 * 3 * 4, dtype=np.float32).reshape(4, 3, 2)
    img = mitk.NativeImage.from_numpy(
        arr,
        spacing=(0.5, 1.0, 2.0),
        origin=(1.0, 2.0, 3.0),
    )

    assert img.shape == arr.shape
    assert img.spacing == (0.5, 1.0, 2.0)
    assert img.origin == (1.0, 2.0, 3.0)

    # round-trip via direct path
    np.testing.assert_array_equal(img.as_numpy(), arr)
    print("  Image.from_numpy (WP-2) OK")


def test_image_array_protocol():
    """WP-5: np.asarray(img) and np.asarray(img, dtype=...)."""
    import mitk
    import numpy as np

    img = mitk.NativeImage.from_numpy(np.zeros((4, 5, 6), dtype=np.uint8))
    arr = np.asarray(img)
    assert arr.shape == (4, 5, 6)
    assert arr.dtype == np.uint8

    arr2 = np.asarray(img, dtype=np.float32)
    assert arr2.dtype == np.float32
    print("  __array__ protocol (WP-5) OK")


def test_image_load_save_roundtrip(tmp_dir=None):
    """WP-4: Image.save() / Image.load() round-trip preserves data + geometry."""
    import mitk
    import numpy as np
    import os
    import tempfile

    arr = np.arange(60, dtype=np.float32).reshape(3, 4, 5)
    img = mitk.NativeImage.from_numpy(arr, spacing=(0.5, 0.7, 1.1))

    with tempfile.TemporaryDirectory() as tmp:
        path = os.path.join(tmp, "out.nrrd")
        img.save(path)

        loaded = mitk.NativeImage.load(path)
        np.testing.assert_array_equal(np.asarray(loaded), arr)
        assert loaded.spacing == (0.5, 0.7, 1.1)

        # IOUtil.load should also produce the same result
        results = mitk.IOUtil.load(path)
        assert len(results) >= 1
        np.testing.assert_array_equal(np.asarray(results[0]), arr)
    print("  IOUtil load/save round-trip (WP-4) OK")


def test_image_constructor():
    """WP-3: Pythonic factory mitk.Image() dispatching by source type."""
    import mitk
    import numpy as np
    import os
    import tempfile

    # Empty
    empty = mitk.Image()
    assert isinstance(empty, mitk.NativeImage)

    # From numpy
    arr = np.zeros((2, 3, 4), dtype=np.uint8)
    img2 = mitk.Image(arr, spacing=(1.0, 2.0, 3.0))
    assert img2.shape == (2, 3, 4)
    assert img2.spacing == (1.0, 2.0, 3.0)

    # From path
    with tempfile.TemporaryDirectory() as tmp:
        path = os.path.join(tmp, "x.nrrd")
        img2.save(path)
        img3 = mitk.Image(path)
        np.testing.assert_array_equal(np.asarray(img3), arr)

    # Unsupported source raises TypeError
    try:
        mitk.Image(42)
    except TypeError:
        pass
    else:
        raise AssertionError("mitk.Image(int) should raise TypeError")
    print("  Image() factory constructor (WP-3) OK")


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

    modules = mitk.get_loaded_modules()

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


def run_tests():
    """Execute all test functions and return exit code."""
    tests = [
        test_import,
        test_create_image,
        test_numpy_roundtrip_direct,
        test_numpy_roundtrip_accessor,
        test_image_geometry,
        test_image_from_numpy,
        test_image_array_protocol,
        test_image_load_save_roundtrip,
        test_image_constructor,
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


# ---------------------------------------------------------------------------
# Driver: discover wheel, create venv, install, run tests in subprocess
# ---------------------------------------------------------------------------

def find_wheel(search_dir):
    """Find the MITK wheel in the given directory."""
    pattern = os.path.join(search_dir, "mitk-*.whl")
    wheels = sorted(glob.glob(pattern))
    if not wheels:
        return None
    return wheels[-1]  # latest by name


def main():
    parser = argparse.ArgumentParser(description="Smoke-test the MITK wheel.")
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Path to MITK-build directory to discover the wheel (default: CWD)",
    )
    parser.add_argument(
        "--run-tests",
        action="store_true",
        help=argparse.SUPPRESS,  # internal: run test functions directly
    )
    args = parser.parse_args()

    # Inner mode: run test functions directly (called from the venv subprocess)
    if args.run_tests:
        return run_tests()

    # Outer mode: discover wheel, create venv, run tests in subprocess
    search_dir = os.path.abspath(args.build_dir) if args.build_dir else os.getcwd()
    wheel_path = find_wheel(search_dir)

    if not wheel_path:
        print(f"Error: no mitk-*.whl found in {search_dir}", file=sys.stderr)
        return 1

    print(f"Wheel: {wheel_path}")

    with tempfile.TemporaryDirectory() as venv_dir:
        venv_path = os.path.join(venv_dir, "venv")

        # Create venv
        subprocess.check_call([sys.executable, "-m", "venv", venv_path])

        # Find the venv Python
        if sys.platform == "win32":
            venv_python = os.path.join(venv_path, "Scripts", "python.exe")
        else:
            venv_python = os.path.join(venv_path, "bin", "python")

        # Install the wheel
        subprocess.check_call(
            [venv_python, "-m", "pip", "install", "--quiet", wheel_path]
        )

        # Run tests inside the venv
        test_script = os.path.abspath(__file__)
        result = subprocess.run([venv_python, test_script, "--run-tests"])
        return result.returncode


if __name__ == "__main__":
    sys.exit(main())
