# Python in MITK {#PythonInMITKPage}

[TOC]

## Overview

There are at least three closely intertwined aspects to consider when working with Python in MITK:

1. **Embedding Python** using the Python C API by including Python.h and linking against the Python library in C++.
2. **Running the Python interpreter** as a separate process, passing command-line arguments.
3. **Exposing MITK functionality to Python** via pyMITK, using SWIG to generate the necessary bindings.

Each of these aspects comes with its own challenges, and their integration imposes certain restrictions and pitfalls that can affect one another. As such, even minor changes to one of these components require careful consideration of the others.

The foundation for making all of this work is to ensure that **a consistent set of Python binaries** is used across all aspects. These binaries and their environment must be **isolated from any system Python** or unrelated Python installations that may exist on the target platform.

Finally, the solution must function reliably across **Windows, Linux, and macOS**.

## Why Conda, Micromamba, official embedded builds, and similar solutions don't work

At first glance, solutions like Conda, Micromamba, or the officially provided embedded Python builds seem like silver bullets. Their advertised features and documentation suggest that robust, ready-to-use options already exist - even multiple ones to choose from.

However, it's a classic "too good to be true" scenario. All of these solutions share a fundamental flaw: **they are not truly isolated**. Instead, they are dynamically linked to various system libraries and other dependencies.

Sooner or later, you'll run into issues like failed builds or runtime crashes due to **binary-incompatible versions of shared dependencies** like Zlib, OpenMP, or others. These libraries are loaded dynamically at runtime and may export the same symbols, leading to symbol collisions, undefined behavior, or crashes. It quickly becomes a mess.

## Standalone Python Builds to the rescue

The [Standalone Python Builds](https://github.com/indygreg/python-build-standalone) project provides fully statically linked Python binaries and patches the CPython build system to use relative paths instead of absolute ones.

With these changes, Python is built from source across a wide matrix of versions, platforms, and build configurations, and distributed via GitHub releases.

These builds are **truly standalone**: you can download, unzip, and run them on any machine without any additional dependencies. They do not interfere with your project’s libraries or depend on system-level Python components, making them ideal for embedding or integration into complex applications like MITK.

The downside of this approach is that these Python builds are significantly larger in file size compared to their dynamically linked counterparts. However, this trade-off is acceptable given the improved isolation, reliability, and portability they provide.

## Standalone Python Builds in MITK

### Superbuild integration

We integrated the Standalone Python Builds as an external project in the MITK superbuild. When `MITK_USE_Python3` is enabled (which is the default in our standard `WorkbenchRelease` configuration), this external project:

1. Downloads the platform-appropriate archive from GitHub,
2. Extracts it into `ep/src/Python3`,
3. Upgrades `pip`, and
4. Pre-installs the `numpy` package, which is required by MITK when Python is enabled.

The project is then installed by copying `ep/src/Python3/` into `MITK-build/python`.

This copy step is crucial: we want to preserve the minimal environment in the `ep` directory for use in packaging and installation. The copy in `MITK-build` is used for running build-tree versions of MITK applications - typically during development. Developers can freely modify this Python environment (e.g., install additional packages) without polluting the clean, minimal base used for distribution.

Since our build system relies on CMake’s `find_package()` to locate external dependencies, we enforce consistency for calls to `find_package(Python3)` by explicitly setting `Python3_ROOT_DIR` to `MITK-build/python` and passing it from the superbuild down to the MITK build itself.

MITK modules can then depend on Python either via the classic MITK syntax `PACKAGE_DEPENDS Python3|Python` or by using the native CMake target like `TARGET_DEPENDS Python3::Python`.

### No more debug builds?

Historically, debug builds with Python have been problematic, as Python packages are rarely distributed in debug variants. As a result, **debug builds are disabled** in MITK when `MITK_USE_Python3` is enabled. Developers should instead use the `RelWithDebInfo` configuration for debugging.

However - and this is a **TODO** - the Standalone Python Builds project also provides debug builds. We need to evaluate whether they make proper Python debug builds viable again in MITK.

## Python modules in MITK

Leaving Python wrapping aside for now, the Python integration in MITK is split into three modules:

1. **MitkPythonHelper**: This module **does not** link against the Python library. It mainly provides utility functions to query paths such as the Python home directory, the Python library, and the Python executable - all of which may differ between the build tree and installed versions of MITK.

2. **MitkPreloadPython**: This module **does not** depend on the Python library at link time. It is automatically loaded with the **MitkCore** module. Its sole purpose is to load the correct Python library at runtime with **global symbol visibility** as early as possible to prevent later loads (e.g., via link-time dependencies of other modules) from restricting symbol visibility or accidentally loading a Python library from a different location.

3. **MitkPython**: This module **does** link against the Python library. It provides the `mitk::PythonContext` class, which serves as the main bridge between MITK and Python.

In most cases, you don't need to interact with the `MitkPreloadPython` module directly. If you want to run the Python interpreter as a separate process, `MitkPythonHelper` is the right dependency. If you want to exchange data (e.g., images) between MITK and Python, you'll want to use `MitkPython`.

## SWIG Wrapping: pyMITK

The Python wrapping of MITK is handled by **SWIG**, resulting in the following outputs:

- The SWIG runtime header:
  - `MITK-build/swigpyrun.h`
- The Python module:
  - `MITK-build/bin/Release/pyMITK.py` (on Windows)
  - `MITK-build/bin/pyMITK.py` (on Linux and macOS)
- The compiled native module:
  - `MITK-build/bin/Release/_pyMITK.pyd` (on Windows)
  - `MITK-build/bin/_pyMITK.so` (on Linux and macOS)

During installation, pyMITK is turned into a proper Python package by copying `pyMITK.py` (renamed to `__init__.py`) and `_pyMITK.[pyd|so]` into the site-packages folder of the standalone Python environment - for example, into `site-packages/pyMITK/`.

When executing a build-tree version of MITK, pyMITK is still found because it resides in the same `bin` folder as the applications. This folder is programmatically added to `sys.path` in `mitk::PythonContext::Activate()`. This ensures that the Python module is discoverable during development, without requiring installation. We cannot install pyMITK during development because the build tree may contain multiple versions of it for different build configurations (e.g., `Release`, `RelWithDebInfo`).

**Note**: Currently, only a small subset of MITK is wrapped to enable data exchange - such as transferring images - between MITK and Python.
