# MitkFileConverter: Exporting and Importing Segmentation Stacks {#MITKFileConverterSegStacksPage}

[TOC]

This guide describes how to use [MitkFileConverter](@ref MITKFileConverterPage) to export a multi-label segmentation as a stack of label images and to import such a stack again, using the MITK segmentation stack format. The format itself is documented in the developer manual, see the [MITK MultiLabel Segmentation Stack format](@ref MITKSegmentationStackFormatPage).

## Prerequisites

- MitkFileConverter, which is part of the MITK Workbench installer (see \ref MITKCmdAppsPage for where to find it).
- A multi-label segmentation in a format MITK can read, e.g. `.nrrd` or DICOM SEG.

The commands below use bash quoting for the JSON option strings. See \ref MITKCmdAppsPage for the equivalent quoting in cmd.exe and PowerShell.

## Use case 1: Export a multi-label segmentation to a stack of label images

### Goal

Convert a multi-label segmentation (e.g. `input_segmentation.nrrd`) into

- a stack of NIfTI images, one per label, and
- a `.mitklabel.json` file describing the stack.

Output example:

```
MySegmentation_label_1.nii.gz
MySegmentation_label_2.nii.gz
MySegmentation.mitklabel.json
```

### Command

```bash
MitkFileConverter -i input_segmentation.nrrd -o ./MySegmentation.nii.gz --output-options '{"Save strategy": "instance", "Instance value": "original"}'
```

### Options

| Option | Value | Description |
|--------|-------|-------------|
| `Save strategy` | `group` (default) | Save one image per label group, `<name>_group_<index>.nii.gz`. |
| `Save strategy` | `instance` | Save one image per label, `<name>_label_<value>.nii.gz`. A group without labels gets a group image. |
| `Instance value` | `original` (default) | With `instance`: voxels carry the label value of the segmentation. |
| `Instance value` | `binary` | With `instance`: voxels carry the value 1 (binary masks). |

## Use case 2: Import a stack and convert it to a multi-label segmentation

### Goal

Convert a segmentation stack described by a `.mitklabel.json` file into a single `.nrrd` file storing the entire multi-label segmentation.

Input example:

```
MySegmentation_label_1.nii.gz
MySegmentation_label_2.nii.gz
MySegmentation.mitklabel.json
```

### Command

```bash
MitkFileConverter -i ./MySegmentation.mitklabel.json -o ./output_segmentation.nrrd
```

The reader combines the label images according to the metadata in the JSON file. The result is a standard MITK multi-label segmentation.

## Notes

- MITK writes the stack images as NIfTI (`.nii.gz`). When reading, the stack images can have any format ITK reads, e.g. `.nii.gz` or `.nrrd`.
- Without `--output-options`, MitkFileConverter writes one image per label group (`MySegmentation_group_0.nii.gz`, ...) instead of one per label.
