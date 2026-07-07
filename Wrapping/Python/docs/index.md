# mitk: Python bindings for MITK

The `mitk` package exposes the data structures and core utilities of the
[Medical Imaging Interaction Toolkit (MITK)](https://www.mitk.org) to Python.
It ships as a self-contained wheel that bundles the compiled MITK runtime
together with its native dependencies (ITK, VTK, CppMicroServices, DCMTK), so
you can `pip install` it and start working with medical images without
building MITK from source.

```python
import mitk
import numpy as np

img = mitk.Image(np.zeros((64, 64, 64), dtype=np.float32), spacing=(1.0, 1.0, 2.5))
img.save("output.nrrd")

loaded = mitk.Image("output.nrrd")
print(loaded.shape, loaded.spacing, loaded.origin)
```

## What this site covers

This site documents the **Python surface** of MITK: classes, functions,
NumPy interop, file I/O, geometry, properties. The wider MITK stack (C++
modules, plugins, the application framework, the BlueBerry plugin system)
is documented separately at
[docs.mitk.org/2026.06/](https://docs.mitk.org/2026.06/). Think of this
site as the Python facade on that stack.

```{toctree}
:maxdepth: 2
:caption: Getting started

installation
getting_started
```

```{toctree}
:maxdepth: 2
:caption: User guide

user_guide/images
user_guide/geometry
user_guide/io
user_guide/properties
user_guide/mxn_layout
```

```{toctree}
:maxdepth: 2
:caption: API reference

api/index
```

## See also

- [docs.mitk.org/2026.06/](https://docs.mitk.org/2026.06/): the full MITK
  C++ reference and user manual.
- [www.mitk.org](https://www.mitk.org): the project homepage.
