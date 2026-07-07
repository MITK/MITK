---
paths:
  - Wrapping/Python/**/*.h
  - Wrapping/Python/**/*.cpp
  - Wrapping/Python/**/*.py
  - Wrapping/Python/**/*.cmake
  - Wrapping/Python/**/CMakeLists.txt
---

# Python bindings

The bindings live in `Wrapping/Python/mitk/` and use pybind11.

## Entry points

- `Module.cpp` - `PYBIND11_MODULE` registration.
- `Image.cpp`, `Geometries.cpp`, `Points.cpp` - per-type bindings.
- `SmartPointer.h` - integration between pybind11 and
  `itk::SmartPointer`.

## Building a Python wheel

Wheel builds require:

- `MITK_BUILD_CONFIGURATION=PythonWheel` - headless (no Qt or
  BlueBerry).
- `Wrapping/Python/wheel/build_wheel.py` as the driver.
- A platform-specific delocator (`auditwheel` / `delocate` /
  `delvewheel`) to bundle native dependencies.

Debug builds are incompatible with `MITK_USE_Python3=ON`; build the
wheel in Release.

### Wheel version

By default the version comes from git: the tag at HEAD, or
`<base>+g<shorthash>` for an untagged commit. That `+g...` suffix is
a PEP 440 local version, which PyPI rejects on upload.

To pin an explicit, uploadable version, set `MITK_WHEEL_VERSION` at
configure time (`-DMITK_WHEEL_VERSION=2026.6.dev1`) or pass
`--version` to `build_wheel.py`. Empty/unset falls back to the git
logic, so a blank CI parameter is a no-op. The chosen version drives
the wheel filename, METADATA, and the package `__version__` alike.

## Copyright header

Every new `.py` file must start with the header at
`Templates/copyright_header.txt`.
