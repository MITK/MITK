# MITKFileConverter: Exporting and Importing MultiLabel Segmentation Stacks {#MITKFileConverterSegStacksPage}

This guide describes how to use the [MitkFileConverter](@ref MITKFileConverterPage) to **export** and **import** multilabel segmentations using the MITK segmentation stack format.

## Prerequisites

Ensure you have:
- A working build of MITK including the [MitkFileConverter](@ref MITKFileConverterPage) tool.
- Input segmentation files in `.nrrd` or DICOM (`.dcm`) format.

---

## Use Case 1: Export a Multilabel Segmentation to a Stack of Label Images (NIfTI)

### Goal

Convert a `MultiLabelSegmentation` (e.g., `input.nrrd`) into:
- A stack of NIfTI images (one per label).
- A `.mitklabel.json` file describing the stack.

### Output Example

```
MySegmentation_Label_1.nii.gz
MySegmentation_Label_2.nii.gz
MySegmentation.mitklabel.json
```

### Command

For Bash / Linux / macOS / WSL (Unix-like shells) / PowerShell:
```
MITKFileConverter \
  -i input_segmentation.nrrd \
  -o ./MySegmentation.nii.gz \
  --output-options '{"Save strategy": "instance", "Instance value": "original"}'
```

For Windows Command Prompt (cmd.exe):
```
MITKFileConverter \
  -i input_segmentation.nrrd \
  -o ./MySegmentation.nii.gz \
  --output-options "{""Save strategy"": ""instance"", ""Instance value"": ""original""}"
```

### Options

Option | Value | Description
------ | ----- | -----------
`Save strategy` | `instance` | Save one image per label
`Instance value` | `original` | Use the label’s value in the image (not binary masks)

You can also use:
- `Instance value` = `binary` to export binary masks (value = 1)

---

## Use Case 2: Import a Stack and Convert to a Multilabel Segmentation (.nrrd)

### Goal

Convert a segmentation stack described by a `.mitklabel.json` file into a single `.nrrd` file storing the entire `MultiLabelSegmentation`.

### Input Example

```
MySegmentation_Label_1.nii.gz
MySegmentation_Label_2.nii.gz
MySegmentation.mitklabel.json
```

### Command

```
MITKFileConverter \
  -i ./MySegmentation.mitklabel.json \
  -o ./output_segmentation.nrrd
```

### Notes

- The converter will auto-detect and combine label images based on the `.json` metadata.
- The output will be a standard MITK-compatible multilabel `.nrrd`.

---

## Tips & Tricks

- You can use `.nii.gz`, `.nrrd`, or other ITK-supported formats.
- To export using **group images** instead of per-label masks:

```
--output-options "Save strategy"="group"
```

---

## Reference

See the [MITK MultiLabel Segmentation Stack Format README](@ref MITKSegmentationStackFormatPage) for more information about JSON structure, label properties, and file mappings.
