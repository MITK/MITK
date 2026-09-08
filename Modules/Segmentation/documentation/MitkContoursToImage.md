# MitkContoursToImage {#MitkContoursToImagePage}

[TOC]

## Overview

MitkContoursToImage rasterizes contours into images. Typical inputs are DICOM RTSTRUCT files or MITK contour model sets. Each contour set is drawn into the voxel grid of a reference image and written as a binary mask, as a single-label segmentation, or, together with all other contour sets of the input, as one multi-label segmentation.

Use it to turn radiotherapy structure sets into segmentations that MITK, or any tool reading NRRD or NIfTI, can process. If you already have masks or surfaces and want to merge them into one multi-label segmentation, use [MitkConvertToMultiLabelSegmentation](@ref MitkConvertToMultiLabelSegmentationPage) instead.

## Usage

```bash
MitkContoursToImage -i <contours> -r <reference image> -o <output> [-f binary|label|multilabel]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | File containing one or more contour sets, e.g. a DICOM RTSTRUCT file or an MITK contour model set (`.cnt_set`). |
| `--reference` | `-r` | Image | Reference image that defines the geometry (size, spacing, orientation) of the output. |
| `--output` | `-o` | Image | Output file path. Used as a base name if more than one file is written, see Details. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--format` | `-f` | String | `binary` | Output format: `binary`, `label`, or `multilabel`. |

There is no `--help` argument. Running the app without arguments prints the help text.

## Details

### Output formats

- `binary`: every contour set becomes an unsigned char image with value 1 inside the contours and 0 outside.
- `label`: every contour set becomes an MITK multi-label segmentation with a single label (value 1). The label takes name and color from the contour set if available.
- `multilabel`: all contour sets become labels of one multi-label segmentation. Each contour set is placed in its own group, labels get consecutive values in the order of the input, and name and color are taken from the contour set.

### Output file names

With `binary` or `label` format and more than one contour set in the input, one file per contour set is written. The name of the contour set is inserted before the file extension: `<output stem>_<contour set name><extension>`. Slashes in names are replaced by underscores, nameless contour sets are numbered `nameless_0`, `nameless_1`, and so on. With a single contour set, or with the `multilabel` format, the output path is used as given. Missing parent directories of the output are created.

### Input handling

Every object in the input file that is not a contour set is skipped with a warning. A contour set that does not intersect the reference image yields an empty mask; with the `label` and `multilabel` formats an empty label is created for it and a warning is printed. Loading the reference image and the contours, as well as writing, use the readers and writers of MITK, so the formats of [MitkFileConverter](@ref MITKFileConverterPage) apply.

## Examples

### Convert an RTSTRUCT into one multi-label segmentation

```bash
MitkContoursToImage -i RS.1.2.840.dcm -r planning_ct.nrrd -o structures.nrrd -f multilabel
```

All structures of the RTSTRUCT are rasterized on the grid of the planning CT and written as a single multi-label segmentation in which each structure is a label with its own group.

### One binary mask per structure

```bash
MitkContoursToImage -i RS.1.2.840.dcm -r planning_ct.nrrd -o mask.nii.gz
```

Writes `mask_PTV.nii.gz`, `mask_Heart.nii.gz`, and so on, one unsigned char mask per structure.

### Rasterize an MITK contour model set

```bash
MitkContoursToImage -i liver.cnt_set -r ct.nrrd -o liver_seg.nrrd -f label
```

The contour set is converted into a segmentation with one label named after the contour set.

## Exit codes

| Code | Meaning |
|------|---------|
| 0 | All contour sets were converted. |
| 1 | Invalid arguments, an exception occurred, or at least one contour set could not be converted (the others are still written). |
