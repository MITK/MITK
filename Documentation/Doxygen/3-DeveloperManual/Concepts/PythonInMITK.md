# Python in MITK {#PythonInMITKPage}

[TOC]

## Overview

There are at least three closely intertwined aspects to consider when working with Python in MITK:

1. **Embedding Python** using the Python C API by including Python.h and linking against the Python library in C++.
2. **Running the Python interpreter** as a separate process, passing command-line arguments.
3. **Exposing MITK functionality to Python** via the mitk Python module, using pybind11 to generate the necessary bindings.

Each of these aspects comes with its own challenges, and their integration imposes certain restrictions and pitfalls that can affect one another.
As such, even minor changes to one of these components require careful consideration of the others.

The foundation for making all of this work is to ensure that **a consistent set of Python binaries** is used across all aspects.
These binaries and their environment must be **isolated from any system Python** or unrelated Python installations that may exist on the target platform.

Finally, the solution must function reliably across **Windows, Linux, and macOS**.

## Why Conda, Micromamba, official embedded builds, and similar solutions don't work

At first glance, solutions like Conda, Micromamba, or the officially provided embedded Python builds seem like silver bullets.
Their advertised features and documentation suggest that robust, ready-to-use options already exist — even multiple ones to choose from.

However, it's a classic "too good to be true" scenario. All of these solutions share a fundamental flaw: **they are not truly isolated**.
Instead, they are dynamically linked to various system libraries and other dependencies.

Sooner or later, you'll run into issues like failed builds or runtime crashes due to **binary-incompatible versions of shared dependencies** between MITK and the libraries of a Python package.
These libraries are loaded dynamically at runtime and may export the same symbols, leading to symbol collisions, undefined behavior, or crashes.

## Standalone Python Builds to the rescue?

The [Standalone Python Builds](https://github.com/indygreg/python-build-standalone) project provides fully statically linked Python binaries and patches the CPython build system to use relative paths instead of absolute ones.

With these changes, Python is built from source across a wide matrix of versions, platforms, and build configurations, and distributed via GitHub releases.

These builds are **fully standalone**: just download, unzip, and run them on any machine, with no additional dependencies.
They do not interfere with your project’s libraries or depend on system-level Python components, making them ideal for embedding or integration into complex applications like MITK.

The downside of this approach is that these Python builds are significantly larger in file size compared to their dynamically linked counterparts.
However, this trade-off is acceptable given the improved isolation, reliability, and portability they provide.

## Standalone Python Builds in MITK

### Superbuild integration

We integrated the Standalone Python Builds as an external project in the MITK superbuild.
When `MITK_USE_Python3` is enabled (which is the default in our standard `WorkbenchRelease` configuration), this external project:

1. Downloads the platform-appropriate archive from GitHub,
2. Extracts it into `ep/src/Python3`,
3. Upgrades `pip`, and
4. Pre-installs the `numpy` package, which is required by MITK when Python is enabled.

The project is then installed by copying `ep/src/Python3/` into `MITK-build/python`.

Since our build system relies on CMake’s `find_package()` to locate external dependencies, we enforce consistency for calls to `find_package(Python3)` by explicitly setting `Python3_ROOT_DIR` to `MITK-build/python` and passing it from the superbuild down to the MITK build itself.

MITK modules can then depend on Python either via the classic MITK syntax `PACKAGE_DEPENDS Python3|Python` or by using the native CMake target like `TARGET_DEPENDS Python3::Python`.

### No more debug builds?

Historically, debug builds with Python have been problematic, as Python packages are rarely distributed in debug variants.
As a result, **debug builds are disabled** in MITK when `MITK_USE_Python3` is enabled.
Developers should instead use the `RelWithDebInfo` configuration for debugging.

## Python modules in MITK

Leaving Python wrapping aside for now, the Python integration in MITK is split into three modules:

1. **MitkPythonHelper**: This module **does not** link against the Python library.
It primarily offers utility functions for retrieving paths such as the Python home directory, the Python library, and the Python executable — all of which can differ between the build tree and installed versions of MITK.
It also includes functions to create, locate, and activate virtual environments.

2. **MitkPreloadPython**: This module **does not** depend on the Python library at link time.
It is automatically loaded with the **MitkCore** module. Its sole purpose is to load the correct Python library at runtime with **global symbol visibility** as early as possible to prevent later loads (e.g., via link-time dependencies of other modules) from restricting symbol visibility or accidentally loading a Python library from a different location.
It also sets and unsets a few environment variables to ensure a clean and functioning Python environment.

3. **MitkPython**: This module **does** link privately against the Python library.
It provides the `mitk::PythonContext` class, which acts as the primary bridge between MITK and Python.

In most cases, you won't need to interact with the `MitkPreloadPython` module directly.
To run the Python interpreter as a separate process, use `MitkPythonHelper`.
To exchange data (e.g., images) between MITK and Python, use `MitkPython`.

## Python Wrapping: The mitk Python module

The Python wrapping of MITK is handled by pybind11.
The bindings are defined in `Wrapping/Python/mitk/` and compiled into a native extension module (`mitk.cpXYZ-<platform>.pyd` / `.so`).

Currently, the following types and functions are exposed:

| Category | Types / Functions |
|---|---|
| **Image** | `Image` (Python factory function) and `NativeImage` (C++ class) with `new()`, `initialize()`, `from_numpy()`, `load()`, `save()`, `as_numpy()`, `__array__`, geometry properties (`spacing`, `origin`, `direction`, `direction_cosines`, `ndim`, `shape`, `dtype`, `array`, `time_steps`, `time_geometry`), per-time-step accessors (`get_spacing()`, `set_spacing()`, `get_origin()`, `set_origin()`, `get_direction()`, `set_direction()`, `get_geometry()`) |
| **IO** | `IOUtil.load()`, `IOUtil.save()` |
| **Geometry** | `BaseGeometry`, `Geometry3D`, `PlaneGeometry`, `SlicedGeometry3D`, `TimeGeometry` (with `count_time_steps()`, `get_min_time_point()`, `get_max_time_point()`, `get_time_bounds()`, `time_step_to_time_point()`, `time_point_to_time_step()`, `is_valid_time_step()`, `is_valid_time_point()`, `get_geometry_for_time_step()`, `get_geometry_for_time_point()`), `ArbitraryTimeGeometry`, `ProportionalTimeGeometry` |
| **Pixel types** | `PixelType`, `make_pixel_type()` |
| **Points / Vectors** | `Point2D`, `Point3D`, `Vector2D`, `Vector3D` |
| **Exceptions** | `Exception` |
| **CppMicroServices** | `get_loaded_modules()` |

`mitk.Image` is a Python factory function that dispatches by source type (numpy array, file path, or empty). The underlying pybind11 class is also exposed as `mitk.NativeImage` for `isinstance()` checks and direct access to its class methods (`new()`, `from_numpy()`, `load()`).

Basic usage:

```python
import mitk
import numpy as np

# Construct from numpy
arr = np.zeros((64, 64, 64), dtype=np.float32)
img = mitk.Image(arr, spacing=(1.0, 1.0, 2.5))

# In-place modification (default direct, unlocked path)
img.as_numpy(writeable=True)[32, 32, 32] = 1.0

# Read access via the array protocol
print(np.asarray(img)[32, 32, 32])  # 1.0

# Geometry access
print(img.shape, img.spacing, img.origin, img.direction)

# Persistence
img.save("output.nrrd")
loaded = mitk.Image("output.nrrd")
```

By default, `as_numpy()` returns a *direct* numpy view that pins the underlying `mitk.Image` via a smart-pointer capsule but does **not** acquire any read/write lock. This is the preferred mode for in-process work and matches the behavior expected by `numpy.asarray()` and the `__array__` protocol. For workflows that need lock-based concurrency control (e.g. multi-threaded access from C++ and Python at the same time), pass `use_accessor=True` to fall back to the legacy `ImageReadAccessor`/`ImageWriteAccessor`-backed view, which holds the MITK accessor lock until the numpy array is garbage-collected:

```python
arr = img.as_numpy(use_accessor=True, writeable=True)
arr[5, 5, 5] = 7
del arr  # release the write accessor before re-acquiring
```

The bindings are available both within MITK applications (via the embedded Python in `MITK-build/python`) and as a standalone installable wheel (see below).

## Virtual environments

When you instantiate the `mitk::PythonContext` class, you can optionally specify the name of a virtual environment to use.
If the environment doesn’t exist yet, it will be created automatically.
This allows different MITK components — such as segmentation tools — to use isolated virtual environments, enabling them to install dependencies at runtime via pip, for example.

These virtual environments are stored in the `mitk_venvs` folder within a dedicated user-writable location:

- `%LocalAppData%` on Windows
- `$XDG_DATA_HOME` or `$HOME/.local/share` on Linux
- `$HOME/Library/Application Support` on macOS

To avoid interference between multiple MITK versions built or installed on the same machine, we use a hash of the application path of the currently running MITK application as the top-level folder name inside `mitk_venvs`.

Virtual environments created by `mitk::PythonContext` (or the corresponding functions in the `MitkPythonHelper` module) can be listed and managed through the **Python Settings** plugin in MITK.

## Python Wheel

The `mitk` Python module can be packaged as a standalone, redistributable wheel (`mitk-*.whl`).
This allows users to `pip install` the MITK bindings into any compatible Python environment without building MITK from source.

### What is in the wheel?

The wheel bundles:

- The compiled pybind11 extension module (`mitk.cpXYZ-<platform>.pyd` / `.so`)
- All CppMicroServices auto-load modules (IO readers/writers, model fit services, etc.)
- All native library dependencies (ITK, VTK, CppMicroServices, MITK modules, etc.), vendored via a platform-specific delocator

On import, `mitk/__init__.py` sets up the environment so that CppMicroServices auto-loading works transparently — the same IO file formats are available as in a full MITK application.

### Building the wheel

Use the `PythonWheel` build configuration, which is a headless configuration (no Qt, BlueBerry, or plugins) locked to Release builds:

```bash
cmake -S . -B ../MITK-superbuild -DMITK_BUILD_CONFIGURATION=PythonWheel
cmake --build ../MITK-superbuild
cmake --build ../MITK-superbuild/MITK-build
```

The last command builds all MITK modules and then automatically produces the wheel in the `MITK-build/` directory via the `mitk_python_wheel` target, which is included in the default build.

The target:
1. Installs pip packaging dependencies (`wheel` + platform delocator) into the build Python
2. Stages the bindings and auto-load modules via `cmake --install --component wheel`
3. Packs a raw wheel and repairs it with the platform delocator to bundle all native dependencies

The platform delocators are:
- **Windows**: [delvewheel](https://github.com/adang1345/delvewheel) — copies DLLs into `mitk.libs/`
- **Linux**: [auditwheel](https://github.com/pypa/auditwheel) — copies shared libraries into `mitk.libs/` and patches RPATH
- **macOS**: [delocate](https://github.com/matthew-brett/delocate) — copies dylibs into `mitk/.dylibs/` and rewrites load commands

### Testing the wheel

A self-contained smoke test script is provided:

```bash
python Wrapping/Python/wheel/test_wheel.py --build-dir ../MITK-superbuild/MITK-build
```

This automatically creates a temporary virtual environment, installs the wheel, runs the tests, and cleans up.
The tests verify import, image creation, NumPy roundtrip, geometry types, and CppMicroServices auto-loading.

### Standalone usage

The `build_wheel.py` script can also be invoked manually outside of the CMake build:

```bash
python Wrapping/Python/wheel/build_wheel.py --build-dir <MITK-build>
```

The wheel is written to the build directory by default.
Use `--output-dir` to write it elsewhere, or `--skip-repair` to skip the delocator step for debugging.

## Quirks

As mentioned at the beginning, Python integration is a complex and sometimes fragile feature that can easily break in certain scenarios.
In particular, differences between our supported platforms—Windows, Linux, and macOS—can be challenging.
This section collects a few non-obvious quirks.

### Preloading the Python library on macOS

While we explictly unset the `PYTHONHOME` environment variable on Windows and Linux before preloading the Python library, on macOS we must explicitly set it.

### OpenMP on macOS

While Python Standalone Builds have proven to be the most reliable and isolated solution for us, we have encountered conflicts between the OpenMP libraries used by MITK and certain Python packages on macOS.
To avoid these issues, OpenMP is automatically disabled on macOS whenever Python is enabled in the build system.

### Python tests on macOS

On macOS, the test driver for the `MitkPython` module attempts to load the Python library before the `MitkPreloadPython` autoload-module has a chance to load it as intended.
This premature loading fails because, in Standalone Python Builds, library paths on macOS are prefixed with `/install/lib/`.
To work around this, we use macOS’s `DYLD_INSERT_LIBRARIES` feature to manually preload the Python library by setting the `ENVIRONMENT` test property for all Python tests.
As a result, the tests run correctly via CTest, but still abort when executed directly.

### Converting Python into a framework for packaging on macOS

To sign an application bundle on macOS with `codesign`, the bundle must follow certain rules regarding the location and format of its binaries.
Therefore, we convert the `python` directory from `MITK-build` into `Python.framework` for packaging.
This conversion is handled in the `MITK-build/FixMacOSInstaller.cmake` script, which CPack executes as a post-build step.

### Importing mitk in the Python interpreter of an installed MITK on macOS

We are using CMake's `BundleUtilities` to create application bundles on macOS.
Unfortunately, it rewrites all library dependency paths to start with `@executable_path/../MacOS`, which works fine for executables in the usual `Contents/MacOS` folder of an app bundle.
However, this breaks when the Python interpreter in `Contents/Frameworks/Python.framework/Versions/A/bin` tries to load the dependencies of the `mitk` package.

To fix this, we adjust the runtime dependency paths of the `mitk` package to use an `@loader_path` approach in the `FixMacOSInstaller.cmake` script, which runs automatically after `fixup_bundle()` has finished modifying all paths.
This fix currently does not cover autoload-modules, which is why they cannot be loaded in this scenario.
Running the Python interpreter as subprocess of an MITK application, however, will load autoload-modules correctly.
