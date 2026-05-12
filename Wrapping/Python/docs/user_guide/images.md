# Images

`mitk.Image` is the central data type in the bindings. It wraps the C++
`mitk::Image` class and supports up to four dimensions (three spatial plus
time), arbitrary numeric pixel types, and multi-component pixels (RGB(A),
vector fields).

## Construction

Three ways to construct an image, all dispatched through the same name:

```python
import mitk
import numpy as np
from pathlib import Path

# 1. Empty image. Call initialize() before use.
img = mitk.Image()
img.initialize("float32", dimensions=[64, 64, 64])
img.initialize("uint8", dimensions=[256, 256, 32], channels=3)

# 2. From a NumPy array.
arr = np.zeros((64, 64, 64), dtype=np.float32)
img = mitk.Image(arr)
img = mitk.Image(arr, spacing=(1.0, 1.0, 2.5))
img = mitk.Image(arr, spacing=(1.0, 1.0, 2.5),
                       origin=(0.0, 0.0, -100.0),
                       direction=np.eye(3))

# 3. From a file path (str or pathlib.Path).
img = mitk.Image("input.nrrd")
img = mitk.Image(Path("input.nrrd"))
```

Equivalent named factory methods are available:

```python
img = mitk.Image.from_numpy(arr, spacing=(1.0, 1.0, 2.5))
img = mitk.Image.load("input.nrrd")
```

## Properties

Read-only NumPy-style introspection:

| Attribute | Meaning |
|---|---|
| `img.ndim` | Number of spatial dimensions. |
| `img.shape` | Tuple of dimension sizes, in NumPy order. |
| `img.dtype` | NumPy dtype matching the MITK pixel type. |
| `img.array` | Read-only NumPy view (zero-copy). |
| `img.spacing`, `img.origin`, `img.direction` | Geometry of time step 0. |
| `img.time_steps` | Number of time steps. |
| `img.time_geometry` | Full `TimeGeometry`. |

## NumPy interop

```python
# Read-only: array protocol; zero-copy.
arr = np.asarray(img)
read_only_view = img.as_numpy(writeable=False)
read_only_view = img.array

# Writable: explicit.
writable_view = img.as_numpy(writeable=True)
writable_view[32, 32, 32] = 1.0
```

### Direct vs. accessor-backed views

`as_numpy()` defaults to a **direct** view: zero-copy, no lock, the view
pins the image alive via a smart-pointer capsule. Pass `use_accessor=True`
to get an `ImageReadAccessor` / `ImageWriteAccessor`-backed view, which
holds MITK's read/write lock until the NumPy array is garbage-collected.

```python
arr = img.as_numpy(use_accessor=True, writeable=True)
arr[5, 5, 5] = 7
del arr  # release the write accessor before re-acquiring
```

Pick the accessor-backed view when other threads (typically C++) may be
writing concurrently. For single-threaded Python code, the direct view is
faster and simpler.

## Time-resolved data

For 4D images, `time_step` selects the time index in `as_numpy()`:

```python
for t in range(img.time_steps):
    frame = img.as_numpy(time_step=t)
    process(frame)
```

Per-time-step geometry is exposed through the geometry accessors:

```python
geom_t0 = img.get_geometry(0)
img.set_spacing((0.8, 0.8, 1.5), time_step=0)
```

See {doc}`geometry` for the full geometry API.

## Persistence

```python
img.save("output.nrrd")
# or
mitk.IOUtil.save(img, "output.nrrd")
```

See {doc}`io` for the I/O surface.
