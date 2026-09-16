# MitkConvertToMultiLabelSegmentation {#MitkConvertToMultiLabelSegmentationPage}

[TOC]

## Overview

MitkConvertToMultiLabelSegmentation converts one or more images, surfaces, or contours into a single MITK multi-label segmentation. Label images contribute one label per distinct pixel value, surfaces and contours are rasterized into binary masks. All inputs are combined into one segmentation whose geometry is taken from a reference image, and conflicting label values are remapped automatically so that every label value is unique in the result.

Use it to turn plain label masks (e.g. the output of an external segmentation tool), surface meshes (`.stl`, `.vtp`, ...), or MITK contour files into a segmentation that the MITK Workbench segmentation tools understand, or to merge several such inputs into one segmentation. For a pure format conversion of an existing segmentation use [MitkFileConverter](@ref MITKFileConverterPage).

## Usage

```bash
MitkConvertToMultiLabelSegmentation -i <input1> [<input2> ...] -o <output> [-r <reference>] [-g]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--inputs` | `-i` | String list | Paths to the input files (images, surfaces, or contours) to convert. |
| `--output` | `-o` | File | Path of the output multi-label segmentation. The extension selects the output format. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--reference` | `-r` | File | | Reference image that defines the geometry of the output. Only needed if no image is among the inputs. |
| `--groups` | `-g` | Flag | | Put every input into its own label group. By default all labels are merged into one group. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Supported inputs

Every input file is loaded with the MITK DICOM reader preferred (`MITK DICOM Reader v2 (autoselect)`). Only the first data object loaded from a file is used; if a file yields several objects (e.g. a DICOM folder with several series) the others are ignored. The object must be an image (`mitk::Image`), a surface (`mitk::Surface`), a contour model, or a contour model set; any other type aborts the conversion with an error.

### Reference geometry

The output segmentation is initialized with the geometry of the reference image:

- If `--reference` is given, that image is the reference.
- Otherwise the reference is the input image with the largest geometry, i.e. the one whose geometry contains the geometries of all other input images. If there is no image among the inputs, the app aborts with an error.

Every input image must either fit into the reference geometry or contain it (sub-geometry check with MITK's default tolerances); otherwise the conversion is aborted with `Geometry mismatch`. Surfaces and contours are not checked, they are rasterized into the reference geometry as they are.

### Label extraction and naming

- Image inputs: the pixel values are cast to the label value type (`unsigned short`). Values above `mitk::Label::MAX_LABEL_VALUE` abort the conversion. Every distinct non-zero value becomes one label; value 0 is background. Both 3D and 3D+t images are supported: for a dynamic image the labels are collected over all time steps and the content of every time step is transferred.
- Surface inputs are rasterized with `mitk::SurfaceToImageFilter`, contour inputs with `mitk::ContourModelSetToImageFilter`. Each yields one binary mask with the label value 1.

Label names depend on the grouping mode:

| Input type | Default (single group) | `--groups` |
|------------|------------------------|------------|
| Image | `<file name> Value <original value>` | `Value <original value>` |
| Surface, contour | `<file name>` | `<file name>` |

`<file name>` is the file name including its extension, e.g. `liver.stl`. Label names are not made unique; passing the same file twice results in two labels with the same name.

### Label values and collisions

Inputs are processed in the given order. A label keeps its original value unless that value is already used by a previously processed label of the segmentation; in that case it receives the smallest value above the currently highest used value. The label name always refers to the original value (`Value 1`), even if the pixel value in the output was remapped. If no free value is left (all 65535 values are used) the app aborts with an error and suggests splitting the inputs into several conversions.

Label values are unique over all groups. Every label gets a color from the MITK label color scheme.

### Grouping and overlaps

By default all labels end up in group 0, which has no name. With `--groups`, the first input goes to group 0 and every further input to a new group; each group is named after the file name (with extension) of its input.

Labels are created locked, and the content transfer respects locks within a group. In the default single-group mode this means that where two inputs overlap, the pixel keeps the label of the input that was processed first. With `--groups` the inputs live in separate groups and overlaps are preserved.

### Output

The result is saved through the MITK I/O infrastructure; use `.nrrd` for the native MITK segmentation format (see also [MitkFileConverter](@ref MITKFileConverterPage) for other output options such as label stacks). On success the app prints the number of created groups and labels and exits with 0; any error exits with 1.

## Examples

### Convert a single binary mask

```bash
MitkConvertToMultiLabelSegmentation -i tumor.nii.gz -o tumor.nrrd
```

The mask becomes a segmentation with one group and one label named `tumor.nii.gz Value 1` with value 1.

### Merge two masks into one group

```bash
MitkConvertToMultiLabelSegmentation -i liver.nii.gz spleen.nii.gz -o organs.nrrd
```

Both masks contain the value 1. `liver.nii.gz Value 1` keeps value 1, `spleen.nii.gz Value 1` collides and is remapped to value 2. Where the masks overlap, the pixel stays liver.

### Rasterize surfaces into separate groups

```bash
MitkConvertToMultiLabelSegmentation -i liver.stl spleen.stl kidney.stl -r ct.nrrd -o organs.nrrd -g
```

No image is among the inputs, so `ct.nrrd` provides the geometry. Each surface is rasterized into its own group named `liver.stl`, `spleen.stl`, and `kidney.stl`, with one label each (`liver.stl`, value 1; `spleen.stl`, value 2; `kidney.stl`, value 3). Overlapping organs are kept because they live in different groups.

### Convert a multi-value label image

```bash
MitkConvertToMultiLabelSegmentation -i atlas_labels.nrrd -o atlas.nrrd
```

An image with the values 1, 2, and 3 becomes one group with the labels `atlas_labels.nrrd Value 1`, `atlas_labels.nrrd Value 2`, and `atlas_labels.nrrd Value 3`; the pixel values stay 1, 2, and 3.
