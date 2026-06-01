# API reference

Auto-generated from the in-source docstrings of every binding. Use the
search box (top right) to jump straight to a class or function.

## Image and pixel types

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.Image
   mitk.PixelType
   mitk.make_pixel_type
```

## File I/O

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.IOUtil
   mitk.get_loaded_modules
```

## Geometry

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.BaseGeometry
   mitk.Geometry3D
   mitk.PlaneGeometry
   mitk.SlicedGeometry3D
   mitk.TimeGeometry
   mitk.ArbitraryTimeGeometry
   mitk.ProportionalTimeGeometry
```

## Points, vectors, color

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.Point2D
   mitk.Point3D
   mitk.Vector2D
   mitk.Vector3D
   mitk.Color
```

## Properties

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.BaseProperty
   mitk.BoolProperty
   mitk.IntProperty
   mitk.FloatProperty
   mitk.DoubleProperty
   mitk.StringProperty
   mitk.ColorProperty
   mitk.TemporoSpatialStringProperty
   mitk.PropertyList
   mitk.PropertyKeyPath
   mitk.DICOMTag
   mitk.DICOMTagPath
   mitk.find_indexed_properties
   mitk.property_from_dict
   mitk.property_view.PropertyView
```

## Multi-label segmentation

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.MultiLabelSegmentation
   mitk.Label
   mitk.LabelGroup
   mitk.LabelVector
   mitk.AlgorithmType
   mitk.MergeStyle
   mitk.OverwriteStyle
   mitk.DICOMCodeSequence
   mitk.DICOMCodeSequenceWithModifiers
   mitk.transfer_labels
   mitk.transfer_labels_at_time_step
   mitk.split_label_value_mapping_by_source_and_target_group
```

## Exceptions

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.Exception
   mitk.PropertyNotOwnedError
```

## MxN layout DSL

The `mitk.mxn.layout` subpackage is a typed, transport-free Python DSL
for constructing, inspecting, transforming, and round-tripping the
multi-widget layout documents that drive `QmitkMxNMultiWidget`. See
{doc}`../user_guide/mxn_layout` for the narrative tour.

### Model

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.mxn.layout.MxNLayoutDocument
   mitk.mxn.layout.Split
   mitk.mxn.layout.LayoutWindow
   mitk.mxn.layout.Group
   mitk.mxn.layout.Link
   mitk.mxn.layout.MxNWindowSelector
```

### Builders

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.mxn.layout.grid
   mitk.mxn.layout.three_up
   mitk.mxn.layout.two_rows_each_direction
   mitk.mxn.layout.single_window
```

### Presets

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.mxn.layout.preset
   mitk.mxn.layout.list_presets
```

### File I/O

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.mxn.layout.load_preset
   mitk.mxn.layout.save_preset
```

### Vocabulary

```{eval-rst}
.. autosummary::
   :toctree: generated/

   mitk.mxn.layout.LinkDimension
   mitk.mxn.layout.ViewDirection
```
