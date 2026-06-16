# mitk: Python bindings for MITK

The `mitk` package exposes the data structures and core utilities of the
[Medical Imaging Interaction Toolkit (MITK)](https://www.mitk.org) to Python.
It ships as a self-contained wheel that bundles the compiled MITK runtime
together with its native dependencies (ITK, VTK, CppMicroServices, DCMTK), so
you can `pip install` it and start working with medical images without
building MITK from source.

## Installation

```
pip install mitk
```

## Quick start

```python
import mitk
import numpy as np

img = mitk.Image(np.zeros((64, 64, 64), dtype=np.float32), spacing=(1.0, 1.0, 2.5))
img.save("output.nrrd")

loaded = mitk.Image("output.nrrd")
print(loaded.shape, loaded.spacing, loaded.origin)
```

## Documentation

The Python API (classes, functions, NumPy interop, file I/O, geometry,
properties) is documented at
[docs.mitk.org/python/latest/](https://docs.mitk.org/python/latest/). The wider
MITK stack (C++ modules, plugins, the application framework) is documented at
[docs.mitk.org/latest/](https://docs.mitk.org/latest/).

## License

MITK is available under a 3-clause BSD license.
Copyright (c) German Cancer Research Center (DKFZ). All rights reserved.
