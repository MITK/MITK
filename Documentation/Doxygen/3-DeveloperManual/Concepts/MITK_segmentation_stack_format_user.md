# MITK Stacked Segmentation Format {#MITKSegmentationStackFormatPage}

[TOC]

This document explains the **MITK MultiLabel Segmentation Stack Format**, including the structure of image and JSON files. This format allows exporting/importing a multi-label segmentation composed of multiple labeled regions, grouped logically, and enriched with metadata.
There are several reader and writer in MITK that support this format.

## 📦 File Structure Overview

A typical stack includes:
```
MySegmentation/
├── MySegmentation_Group_0.nrrd             # optional group image ( (multiple) label values in one image)
├── MySegmentation_Label_1.nii.gz           # optional label image (per label)
├── ...
└── MySegmentation.mitklabel.json           # required JSON meta file
```

The `.json` file determines how image files are interpreted and combined into a `mitk::MultiLabelSegmentation`.
The images can have any format that is supported by MITK.
**IMPROTANT**: The images of one segmentation stack have to have the same image geometry


## 🧩 How Data is Structured

Segmentations are organized into **label groups**. Each group may:

- Contain one or more labels
- Reference a single **group image** (multi-label voxel values)
- Or contain multiple **label-specific binary masks**

This is configured by the `save_strategy`:
- `group`: Saves one image per group
- `label`: Saves one image per label

MITK handles both variants at import/export.


## 📄 JSON File Structure

### Top-Level Fields

```json
{
  "version": 3,
  "type": "org.mitk.multilabel.segmentation.stack",
  "uid": "optional-unique-id",
  "groups": [ ... ],
  "properties": { ... }
}
```

| Key         | Required | Description                                         |
|-------------|----------|-----------------------------------------------------|
| `version`   | ✅        | Format version (integer)                           |
| `type`      | ✅        | Must be `"org.mitk.multilabel.segmentation.stack"` |
| `uid`       | ❌        | Unique ID for the segmentation                     |
| `groups`    | ✅        | Array of label groups                              |
| `properties`| ❌        | Global image metadata/properties                   |


### Group Structure

Each entry in `"groups"` defines a group of labels:

```json
{
  "name": "Custom Group Name",
  "_file": "./Group_0.nrrd",
  "labels": [ ... ],
  "myCustomGroupProperty": "value"
}
```

| Key      | Required | Description                               |
|----------|----------|-------------------------------------------|
| `name`   | ❌        | Optional group name                       |
| `_file`   | ❌        | Path to the group image file              |
| `labels` | ✅        | Array of labels in the group              |
| *(any)*  | ❌        | Any additional key is stored as a custom property |



### Label Structure

Each label includes visual and semantic metadata, and optionally its own image file:

```json
{
  "name": "Label 1",
  "value": 1,
  "_file": "./Label_1.nii.gz",
  "_file_value": 1,
  "color": [1.0, 0.0, 0.0],
  "opacity": 0.6,
  "locked": true,
  "visible": true,
  "tracking_id": "1",
  "tracking_uid": "uuid",
  "description": "Optional text",
  "DICOM.0062.0002.0062.0008": { ... },
  "custom_property": "value"
}
```

| Key           | Required | Description                                            |
|----------------|----------|--------------------------------------------------------|
| `name`         | ✅        | Label name. When stored as DICOM this will be mapped into the tag `Segment Label (0062,0005)` |
| `value`        | ✅        | Unique label value (*it hase to be unique for the whole segmentation not just the group!*) |
| `_file`         | ❌        | Path to binary label image                             |
| `_file_value`   | ❌        | Voxel value in the image to map to/form `value` on import/export            |
| `color`        | ✅        | Controls UI color — RGB values `[r, g, b]` (0.0–1.0). When stored as DICOM this will be mapped into the tag `Recommended Display CIELab Value (0062,000D)` |
| `opacity`      | ✅        | Opacity (0.0–1.0)                                      |
| `locked`       | ✅        | Controls UI editability — `true` disables editing   |
| `visible`      | ✅        | Visibility in the UI                                   |
| `tracking_id`  | ✅        | Tracking ID (string or number). When stored as DICOM this will be mapped into the tag `TrackingID (0062,0020)` |
| `tracking_uid` | ❌        | Optional unique identifier. When stored as DICOM this will be mapped into the tag `TrackingUID (0062,0021)` |
| `description`  | ❌        | Optional user description. When stored as DICOM this will be mapped into the tag `Segment Description (0062,0006)` |
| *(any)*        | ❌        | Custom label properties (e.g., DICOM metadata, flags)  |

### Meta keys/properties
Some keys on group and label level are not supposed to be converted into regular properties, but the are supposed as meta information that control the way labels are processed/handled.
Those meta keys will not be "imported", thus transformed into properties of the label data structure. Those keys are indicated by a "_" prefix. The most regular seen keys of that type are `_file` and `_file_value`.

## Mapping Image Voxel Values

To **control voxel value interpretation**:

- Use `_file_value` to specify which voxel value in the image corresponds to this label.
- This is especially useful for **binary masks**, where all label voxels are `1`.

### Example

```json
{
  "name": "Label 5",
  "value": 6,
  "_file": "./binary_label_6.nii.gz",
  "_file_value": 1  // voxel value 1 will be mapped to label value 6
}
```

If `_file_value` is omitted, the label `value` must match the voxel value in the image.

## Properties Support

### Global Properties

Saved under `"properties"`:

```json
"properties": {
  "StringProperty": {
    "custom.test.information": "Custom test content"
  },
  "TemporoSpatialStringProperty": {
    "DICOM.0008.0060": {
      "values": [{ "t": 0, "z": 0, "value": "SEG" }]
    },
    ...
  }
}
```

These can store standard metadata (e.g., DICOM attributes) or user-defined content. On this level (which resembles the level of a data instance (like an image)) the properties are stored in the same format like any node or data properties in MITK.

### Label/Group Custom Properties

Labels and groups may contain custom tags/properties.
For simplicity/readablility reasons simple types are directly stored as simple key/value pairs.

Supported types:

- Strings
- Integers
- Floats
- Booleans
- Structured properties (e.g. `TemporoSpatialStringProperty`)

Unknown or unsupported types may cause import failure if not handled correctly.
Structured properties (representing more complex properties (e.g. `TemporoSpatialStringProperty`) are stored in the following structure:

```json
 <property_name>: {
   "type": <property_class_name>,
   "value": <property_content>
  }
```
- <property_name>: Name of the property
- <property_class_name>: Name of the property class that is persisted here
- <property_content>: JSON content of the property

Example for a structured property stored at group or label level:
```json
"DICOM.0062.0002.0062.0008": {
  "type": "TemporoSpatialStringProperty",
  "value": { "values": [{"t": 0, "value": "SEMIAUTOMATIC", "z": 0}]}
}
```

---

## 📚 Real-World Example: Mixed Format

A mixed segmentation stack may contain:

- A group image (`_file`) containing label values (e.g., 1, 2, 3).
- Specific label images (`_file`) with optional `_file_value` mappings.
- One group might not have a `_file` at all, relying entirely on per-label images.

### Example

```json
{
  "groups": [
    {
      "_file": "./Group_0.nrrd",
      "labels": [
        {
          "name": "Bone",
          "value": 1,
          "_file": "./BoneMask.nii.gz",
          "color": [0.8, 0.1, 0.1],
          "locked": true,
          "visible": true,
          "opacity": 0.6,
          "tracking_id": "1"
        },
        {
          "name": "Muscle",
          "value": 2,
          "color": [0.1, 0.8, 0.1],
          "locked": true,
          "visible": true,
          "opacity": 0.6,
          "tracking_id": "2"
        }
      ]
    },
    {
      "name": "Binary Masks",
      "labels": [
        {
          "name": "Vessel",
          "value": 3,
          "_file": "./Vessel.nii.gz",
          "_file_value": 1,
          "color": [0.1, 0.1, 0.8],
          "locked": false,
          "visible": true,
          "opacity": 0.5,
          "tracking_id": "3"
        }
      ]
    }
  ]
}
```

### Explanation

- Group 0 uses a group image and overrides it for label "Bone" via file "./BoneMask.nii.gz".
- If the group image contains also a label of value 1, it will be overwritten by the dedicated definition of "Bone". 
- Group 1 has no group image. It contains binary mask label (pixel/label value in file: 1) with explicit voxel mapping to the label value 3.


## Importing and exporting segmentation stacks

The cmd app [MitkFileConverter](@ref MITKFileConverterPage).offers the possibility to import and export segmentation stacks (also using this json meta file).
For more information about how to do that please visit the [MitkFileConverter segmentation stack guide](@ref MITKFileConverterSegStacksPage).