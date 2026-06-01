# Properties

MITK objects carry typed key/value metadata called *properties*. Examples:
DICOM tags on an image, rendering hints, segmentation labels, arbitrary
user data. The bindings expose properties as a Pythonic mapping on objects
that implement `IPropertyOwner` (notably `mitk.Image`).

## Basic usage

```python
img.properties["DICOM.PatientName"] = "Doe^John"
img.properties["my.custom.key"] = 42

print(img.properties["DICOM.PatientName"])
print(list(img.properties))            # all keys
print(len(img.properties))             # number of properties
print("DICOM.PatientName" in img.properties)

del img.properties["my.custom.key"]
```

Reading delegates to `get_property()`, which returns Python-native values
(`str`, `bool`, `int`, `float`, an `(r, g, b)` tuple for colors, or a
`mitk.BaseProperty` for unknown types). Writing delegates to
`set_property()`, which raises `mitk.PropertyNotOwnedError` for read-only
(provided but not owned) properties.

## Owned vs. provided

A property can be **owned** (writable on this object) or **provided**
(visible but read-only, typically inherited from somewhere else). Reading
works for both; writing or deleting only works for owned properties.

```python
img.property_is_owned("DICOM.PatientName")    # True / False
```

Trying to write to a provided property raises:

```python
try:
    img.properties["read.only.key"] = "value"
except mitk.PropertyNotOwnedError as exc:
    print(exc)
```

## Thread safety

`PropertyView` provides no synchronization. If properties are modified
concurrently (for example from a C++ background thread), keys obtained
during iteration may become stale before they are used. Callers that need a
consistent snapshot should copy the view into a dict:

```python
snapshot = dict(img.properties.items())
```

## Property key paths

For nested or namespaced keys, `mitk.PropertyKeyPath` and
`mitk.DICOMTagPath` provide structured key representations.
`PropertyKeyPath` takes a single element and is extended with `/` (like
`pathlib.Path`), `[n]` for an indexed selection, or `["*"]` for a
wildcard:

```python
path = mitk.PropertyKeyPath("DICOM") / "PatientName"
img.set_property(str(path), "Doe^John")

dicom_path = mitk.DICOMTagPath(0x0010, 0x0010)  # (group, element)
```

## Lower-level API

The mapping is implemented by `mitk.property_view.PropertyView`. The
mapping interface (`__getitem__`, `__setitem__`, `__delitem__`,
`__contains__`, `__iter__`, `__len__`) covers most use cases. For
lower-level access, `get_property()`, `set_property()`, `remove_property()`,
and `property_keys` are exposed directly on the object.
