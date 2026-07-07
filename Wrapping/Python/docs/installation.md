# Installation

The `mitk` package is distributed as a platform-specific wheel that bundles
the compiled MITK runtime together with all native dependencies. There is no
need to build MITK from source to use the Python bindings.

## Requirements

- A Python version matching one of the published wheel tags (typically
  CPython 3.10, 3.11, 3.12, or 3.13). The wheel is a binary build pinned
  to a specific CPython ABI; `pip` will refuse to install the wrong tag.
- NumPy 2.0 or newer (installed automatically as a dependency).
- A supported platform: Windows x86_64, Linux x86_64 (glibc 2.39+ /
  manylinux), or macOS (x86_64 and arm64).

## Install

```bash
pip install mitk-python
```

```{note}
The PyPI distribution is named `mitk-python`; the import name stays `mitk`.
At the time of writing, `mitk-python` is not yet on PyPI. The wheel is distributed
through the MITK project's internal channels. Once a public release is
available, the command above will Just Work.
```

To install a wheel built locally or downloaded directly:

```bash
pip install /path/to/mitk_python-<version>-<tags>.whl
```

## Verify the install

```python
import mitk
print(mitk.__version__)
```

If the import succeeds and prints a version string, the wheel is correctly
installed. The wheel bundles auto-load modules (DICOM and image-format
readers/writers) that are activated transparently on import; no further
setup is required.

## Building from source

Building the wheel from source is a developer-side concern; it requires the
full MITK SuperBuild and is documented in the MITK developer manual under
[Python in MITK](https://docs.mitk.org/2026.06/PythonInMITKPage.html). The
short version:

```bash
cmake -S . -B ../MITK-superbuild -DMITK_BUILD_CONFIGURATION=PythonWheel
cmake --build ../MITK-superbuild
```

The SuperBuild build chains into the inner MITK build, which in the
`PythonWheel` configuration runs the `mitk_python_wheel` target as part
of the default build. The resulting wheel lands in
`../MITK-superbuild/MITK-build/`.
