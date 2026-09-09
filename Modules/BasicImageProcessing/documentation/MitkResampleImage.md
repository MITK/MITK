# MitkResampleImage {#MitkResampleImagePage}

[TOC]

## Overview

MitkResampleImage resamples an image to a new voxel spacing, for example to make images of a study isotropic or to bring them to a common resolution before feature extraction. Interpolation method and grid alignment can be chosen.

For masks use [MitkResampleMask](@ref MitkResampleMaskPage), which keeps the result binary. To resample an image onto the grid of another image, or with a registration, use [MitkMapImage](@ref MitkMapImagePage). This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkResampleImage -i <image> -o <output> -x <spacing> -y <spacing> -z <spacing> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | File | Input image. |
| `--output` | `-o` | File | Output image. |
| `--spacing-x` | `-x` | Float | New spacing in x direction (mm). A value of 0 or less keeps the original spacing. |
| `--spacing-y` | `-y` | Float | New spacing in y direction (mm). A value of 0 or less keeps the original spacing. |
| `--spacing-z` | `-z` | Float | New spacing in z direction (mm). A value of 0 or less keeps the original spacing. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--mapping-type` | `-mapping` | Int | 0 | Grid alignment: 0 origin aligned, 1 center aligned, 2 same size. |
| `--interpolator-type` | `-interpolator` | Int | 0 | Interpolation: 0 linear, 1 B-spline (order 3), 2 nearest neighbour, 3 windowed sinc (Hamming), 4 windowed sinc (Welch). |
| `--as-double` | `-double` | Flag | off | Cast the image to double before resampling and write a double image. |
| `--round` | `-round` | Flag | off | Round the resampled values back to the original pixel type. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Grid alignment

- Origin aligned (0): the origin stays where it is, the extent of the output may differ from the input.
- Center aligned (1): the center of the image is preserved.
- Same size (2): the spacing is adjusted so that the output covers the same physical extent as the input.

Out-of-range values fall back to origin aligned (mapping) and linear (interpolator).

### Pixel type

Without `--as-double` the output has the pixel type of the input; interpolated values are truncated for integer images unless `--round` is given.

The first object in the input file is used. If the file cannot be read or does not contain an image, the app reports the error and exits with code 1.

## Examples

### Isotropic 1 mm resampling with linear interpolation

```bash
MitkResampleImage -i ct.nrrd -o ct_1mm.nrrd -x 1 -y 1 -z 1
```

### Change only the slice spacing, B-spline interpolation, rounded result

```bash
MitkResampleImage -i mr.nrrd -o mr_z1.nrrd -x 0 -y 0 -z 1 -interpolator 1 -round
```

### Center aligned resampling with double output

```bash
MitkResampleImage -i pet.nrrd -o pet_2mm.nrrd -x 2 -y 2 -z 2 -mapping 1 -double
```
