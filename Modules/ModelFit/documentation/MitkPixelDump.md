# MitkPixelDump {#MitkPixelDumpPage}

[TOC]

## Overview

MitkPixelDump writes the voxel values of one or more images into a CSV file, one row per voxel. All images must share the same geometry. A mask can restrict the dump to a region of interest. Dynamic images (3D+t) contribute one column per time step.

Use it to inspect voxel values in a spreadsheet or statistics package, for example to compare the parameter maps produced by [MitkGenericFitting](@ref MitkGenericFittingPage) or [MitkMRPerfusion](@ref MitkMRPerfusionPage) with the original signal.

## Usage

```bash
MitkPixelDump -i <image1> [<image2> ...] -o <output.csv> [-m <mask>] [-c <caption1> <caption2> ...]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--inputs` | `-i` | String list | Paths of the images to dump. All images must have the same geometry. |
| `--output` | `-o` | File | Path of the CSV file to write. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--mask` | `-m` | File | | Mask image. Only voxels with a mask value greater than 0 are dumped. Must have the same geometry as the images. |
| `--captions` | `-c` | String list | image paths | Column captions for the images. The number of captions must equal the number of images. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### CSV layout

The first line is the header `x,y,z` followed by one caption per column. Each further line holds the voxel index (x, y, z) and the values of all images at that index, in the order the images were passed. Values are written as floating point numbers. Rows are ordered by x, then y, then z.

For a dynamic image, one column per time step is written. The caption is extended by the time step in brackets, for example `dce.nrrd [0]`, `dce.nrrd [1]`, and so on.

### Geometry checks

The geometry of the first image, or of the mask if given, defines the dumped region. Every other image must have the same spacing, direction and origin (tolerance 1e-6) and must cover that region, otherwise it is excluded from the dump with an error message and the app continues with the remaining images. Loading uses the readers of MITK; for DICOM directories the "MITK DICOM Reader v2 (autoselect)" is preferred.

### Mask handling

If the mask has more than one time step, only the first is used. The mask is used at its own region, so a mask that is a sub-region of the images restricts the dump to that sub-region.

## Examples

### Dump two parameter maps inside a tumor mask

```bash
MitkPixelDump -i ktrans.nrrd ve.nrrd -m tumor.nrrd -o tumor_values.csv -c Ktrans ve
```

Writes a CSV with columns `x,y,z,Ktrans,ve` containing one row for every voxel of the tumor mask.

### Dump the time course of a dynamic image

```bash
MitkPixelDump -i dce.nrrd -o timecourse.csv -m roi.nrrd
```

Each time step of the dynamic image becomes a column `dce.nrrd [t]`, so every row is the full signal curve of one voxel.

### Dump a whole image

```bash
MitkPixelDump -i ct.nrrd -o ct.csv
```

Without a mask, every voxel of the image is written. For large images this produces a very large file.
