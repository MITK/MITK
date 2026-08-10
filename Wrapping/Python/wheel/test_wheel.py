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

"""Smoke tests for the MITK wheel.

Scope: wheel-specific concerns only — that the wheel installs, imports,
exposes its version, and bundles the CppMicroServices auto-load modules
correctly. A single minimal functional check is kept as a belt-and-suspenders
sanity test; comprehensive binding coverage lives in the pytest suite under
``Modules/Python/test/pytest/`` and runs as ``mitkPythonBindingsTest``.

Usage:
  python test_wheel.py --build-dir <MITK-build>     # discover wheel in build dir
  python test_wheel.py                               # discover wheel in CWD
  python test_wheel.py --python <interpreter>         # test another CPython series

The script creates a temporary virtual environment, installs the wheel,
runs the tests, and cleans up automatically.

A build tree can hold one wheel per CPython series, and each only installs on
the series it was compiled against. The wheel is therefore selected by the ABI
tag of the interpreter under test; pass --wheel to override that choice.
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
    """Wheel installs and exposes version metadata."""
    import mitk
    assert hasattr(mitk, "__version__")
    print(f"  mitk {mitk.__version__} imported successfully")


def test_autoload_modules():
    """Wheel bundles CppMicroServices auto-load modules in the expected layout.

    The auto-load modules (IO readers/writers) live in ``mitk/MitkCore/``
    inside the wheel. If the packaging step drops them or places them
    elsewhere, they won't appear in the loaded-modules list at runtime."""
    import mitk

    modules = mitk.get_loaded_modules()

    # MitkCore is a direct dependency of the bindings.
    assert "MitkCore" in modules, f"MitkCore not loaded. Loaded: {modules}"

    expected_autoloads = [
        "MitkDICOMImageIO",
        "MitkIOExt",
        "MitkMultilabelIO",
    ]

    missing = [m for m in expected_autoloads if m not in modules]
    assert not missing, f"Auto-load modules not loaded: {missing}. Loaded: {modules}"

    print(f"  Auto-load OK ({len(modules)} modules loaded: {', '.join(sorted(modules))})")


def test_sanity_create_image():
    """Minimal functional check: construct an image and read back its shape.

    Comprehensive binding coverage lives in the pytest suite. This single
    test stays here as a last-resort sanity check — if the wheel's native
    extension module itself is broken, this will catch it before we delegate
    deeper verification to the pytest-based tests."""
    import mitk

    img = mitk.Image()
    img.initialize("float32", [64, 64, 64])

    assert img.get_dimension() == 3
    assert img.get_dimension(0) == 64
    print("  Image creation sanity check OK")


def test_property_view_packaged():
    """property_view.py is shipped in the wheel.

    Accessing .properties on an Image triggers ``from mitk.property_view
    import PropertyView``.  If property_view.py is absent from the wheel
    this raises ModuleNotFoundError."""
    import mitk

    img = mitk.Image()
    img.set_property("name", "wheel-test")
    view = img.properties
    assert view["name"] == "wheel-test"
    print("  property_view import and .properties access OK")


def run_tests():
    """Execute all test functions and return exit code."""
    tests = [
        test_import,
        test_autoload_modules,
        test_sanity_create_image,
        test_property_view_packaged,
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

def get_abi(python_exe):
    """Return the CPython ABI tag of the given interpreter, e.g. 'cp313'."""
    code = "import sys; print(f'cp{sys.version_info.major}{sys.version_info.minor}')"
    return subprocess.check_output([python_exe, "-c", code], text=True).strip()


def find_wheel(search_dir, abi):
    """Find the MITK wheel for one ABI tag in the given directory."""
    pattern = os.path.join(search_dir, f"mitk_python-*-{abi}-*.whl")
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
        "--python",
        default=None,
        help="Interpreter to test against; its ABI tag selects the wheel "
             "(default: the interpreter running this script)",
    )
    parser.add_argument(
        "--wheel",
        default=None,
        help="Wheel to test, bypassing discovery",
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
    base_python = args.python or sys.executable
    abi = get_abi(base_python)

    if args.wheel:
        wheel_path = os.path.abspath(args.wheel)
    else:
        search_dir = os.path.abspath(args.build_dir) if args.build_dir else os.getcwd()
        wheel_path = find_wheel(search_dir, abi)

        if not wheel_path:
            print(f"Error: no mitk_python-*-{abi}-*.whl found in {search_dir}",
                  file=sys.stderr)
            return 1

    print(f"Interpreter: {base_python} ({abi})")
    print(f"Wheel: {wheel_path}")

    with tempfile.TemporaryDirectory() as venv_dir:
        venv_path = os.path.join(venv_dir, "venv")

        # Create venv
        subprocess.check_call([base_python, "-m", "venv", venv_path])

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
