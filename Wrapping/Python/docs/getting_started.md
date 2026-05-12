# Getting started

This page walks through the basics of working with `mitk` in Python: creating
an image, going to and from NumPy, reading and writing image files,
inspecting geometry.

## Creating an image

`mitk.Image` is the central data type. It is the bound C++ `mitk::Image`
class, so `isinstance(img, mitk.Image)` works, IDEs can autocomplete on it,
and subclassing works the same way as for any Python class. The constructor
is overloaded by argument type: the same name handles empty construction,
loading from a file, and wrapping a NumPy array.

```python
import mitk
import numpy as np
from pathlib import Path

# Empty image: call initialize() before use.
empty = mitk.Image()
empty.initialize("float32", [64, 64, 64])

# Construct from a NumPy array.
arr = np.zeros((64, 64, 64), dtype=np.float32)
img = mitk.Image(arr, spacing=(1.0, 1.0, 2.5))

# Construct from a file path (str or pathlib.Path).
loaded = mitk.Image("input.nrrd")
loaded = mitk.Image(Path("input.nrrd"))
```

Named factory methods `mitk.Image.from_numpy()` and `mitk.Image.load()` are
also available for callers who prefer to be explicit.

## NumPy interop

A `mitk.Image` exposes its buffer as a NumPy array via two complementary
paths:

```python
# Read-only view via the array protocol.
arr = np.asarray(img)            # zero-copy, read-only
print(arr[32, 32, 32])

# Writable view.
view = img.as_numpy(writeable=True)
view[32, 32, 32] = 1.0
```

By default, `as_numpy()` returns a **direct** view that pins the
`mitk.Image` alive via a smart-pointer capsule but does not acquire any
read/write lock. This is the preferred mode for in-process work and matches
the expectations of `numpy.asarray()` and the `__array__` protocol.

For workflows that need lock-based concurrency control (multi-threaded
access from C++ and Python at the same time), pass `use_accessor=True` to
get a view backed by MITK's `ImageReadAccessor` / `ImageWriteAccessor`. The
accessor holds the MITK read/write lock until the NumPy array is
garbage-collected:

```python
arr = img.as_numpy(use_accessor=True, writeable=True)
arr[5, 5, 5] = 7
del arr  # release the write accessor before re-acquiring
```

## File I/O

The `mitk.Image` constructor and `mitk.Image.save()` cover the common
case:

```python
img = mitk.Image("input.nrrd")
img.save("output.nrrd")
```

The explicit forms route through `mitk.IOUtil`. `IOUtil.load()` returns
a *list* of bound objects (a file may contain more than one, and DICOM
series in particular often expand into several), so unpack the result:

```python
[img] = mitk.IOUtil.load("input.nrrd")
mitk.IOUtil.save(img, "output.nrrd")
```

The wheel bundles auto-loaded readers and writers for the formats MITK
supports natively (NRRD, NIfTI, DICOM, MetaImage, and others).

## Geometry

Every image has a 3D geometry (spacing, origin, direction cosines) and an
optional time geometry for 4D data:

```python
print(img.shape)       # (z, y, x) or (t, z, y, x) for 4D
print(img.spacing)     # (sx, sy, sz)
print(img.origin)      # (ox, oy, oz)
print(img.direction)   # 3x3 direction cosine matrix
print(img.time_steps)  # number of time steps
```

See {doc}`user_guide/geometry` for the full geometry API.

## Properties

MITK objects carry typed key/value metadata accessible as a Pythonic
mapping:

```python
img.properties["DICOM.PatientName"] = "Doe^John"
print(list(img.properties))
del img.properties["temp.scratch"]
```

See {doc}`user_guide/properties` for details on owned vs. provided
properties, key paths, and the `PropertyView` mapping.

## Next steps

- {doc}`user_guide/images` for the full image API.
- {doc}`api/index` for the complete API reference.
