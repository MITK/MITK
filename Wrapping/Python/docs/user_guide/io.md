# File I/O

The `mitk.IOUtil` namespace exposes the file readers and writers that ship
with MITK. The wheel bundles auto-loaded modules for the common formats, so
no further setup is needed beyond importing `mitk`.

## Read

`mitk.Image` accepts a path directly and returns a single image, which is
the convenient form for everyday use:

```python
import mitk

img = mitk.Image("input.nrrd")
img = mitk.Image.load("input.nrrd")
```

`mitk.IOUtil.load()` is the explicit form. It returns a *list* of bound
objects because a file (especially a DICOM series) may produce more than
one, so unpack the result:

```python
[img] = mitk.IOUtil.load("input.nrrd")
```

All three accept `str`, `pathlib.Path`, and any object with a
`__fspath__` method.

## Write

```python
img.save("output.nrrd")
# or
mitk.IOUtil.save(img, "output.nrrd")
```

The output format is inferred from the file extension. To control it
explicitly, pick a path with the desired extension.

## Supported formats

Out of the box, the wheel supports the formats MITK supports natively:

- **NRRD** (`.nrrd`, `.nhdr`)
- **NIfTI** (`.nii`, `.nii.gz`)
- **MetaImage** (`.mha`, `.mhd`)
- **DICOM** (`.dcm`, plus DICOM series via a directory path)
- **Surface meshes**: `.stl`, `.ply`, `.obj`, `.vtk`, `.vtp`
- **Point sets**: `.mps`

Auto-loaded modules are listed at runtime via:

```python
for name in mitk.get_loaded_modules():
    print(name)
```

This is useful when troubleshooting why a particular format is not
recognized: if the corresponding auto-load module is missing, the format
will be unavailable.
