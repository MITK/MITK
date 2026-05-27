# Geometry

MITK images carry a spatial geometry (spacing, origin, direction cosines)
and an optional time geometry that maps time steps to time points. The
bindings expose the geometry both via convenience accessors on `mitk.Image`
and via the full geometry classes.

## Accessors on `mitk.Image`

The common case is to read and write spacing, origin, and direction for
time step 0:

```python
print(img.spacing)            # (sx, sy, sz)
print(img.origin)             # (ox, oy, oz)
print(img.direction)          # 3x3 direction cosine matrix (NumPy array)
print(img.direction_cosines)  # same values, flat 9-element tuple (row-major)

img.spacing = (0.8, 0.8, 1.5)
img.origin  = (0.0, 0.0, -100.0)
```

For 4D images, per-time-step accessors are also available:

```python
img.get_spacing(time_step=0)
img.set_spacing((0.8, 0.8, 1.5), time_step=0)
img.get_origin(time_step=1)
img.get_direction(time_step=0)
img.set_direction(np.eye(3), time_step=0)
```

## Full geometry objects

For lower-level access, the full geometry classes are exposed:

```python
geom = img.get_geometry(time_step=0)
print(geom)  # mitk.BaseGeometry subclass

tg = img.time_geometry
print(tg.count_time_steps())
print(tg.get_min_time_point(), tg.get_max_time_point())
```

| Class | Purpose |
|---|---|
| `mitk.BaseGeometry` | Common interface for spatial geometries. |
| `mitk.Geometry3D` | Concrete 3D spatial geometry. |
| `mitk.PlaneGeometry` | Geometry of a 2D plane embedded in 3D. |
| `mitk.SlicedGeometry3D` | A stack of plane geometries. |
| `mitk.TimeGeometry` | Abstract time geometry. |
| `mitk.ArbitraryTimeGeometry` | Per-time-step user-defined time points. |
| `mitk.ProportionalTimeGeometry` | Evenly spaced time points. |

## Time-step / time-point conversion

`TimeGeometry` is the bridge between integer time steps (array indices) and
floating-point time points (the physical time the user sees):

```python
tg = img.time_geometry
tp = tg.time_step_to_time_point(2)
ts = tg.time_point_to_time_step(tp)
assert tg.is_valid_time_step(ts)
assert tg.is_valid_time_point(tp)

bounds = tg.get_time_bounds()        # (min_tp, max_tp)
geom_t1 = tg.get_geometry_for_time_step(1)
geom_t  = tg.get_geometry_for_time_point(tp)
```

For static (3D) images, the time geometry has a single time step covering
the full time range; the conversions still work and return that one step.

## NumPy <-> MITK conventions

MITK stores spacing, origin, and direction in physical (x, y, z) order.
NumPy stores arrays in C (z, y, x) order. The bindings preserve this
distinction: `img.shape` is in NumPy order, but `img.spacing`, `img.origin`,
and `img.direction` are in physical order.
