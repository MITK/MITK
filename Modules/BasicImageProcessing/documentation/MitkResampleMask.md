# MitkResampleMask {#MitkResampleMaskPage}

[TOC]

## Overview

MitkResampleMask resamples a binary mask to a new voxel spacing while keeping it binary. With nearest neighbour interpolation the mask is resampled directly. With any other interpolation method the mask is resampled as a double image and thresholded at 0.5, which preserves the mask boundary better than nearest neighbour resampling.

For grey value images use [MitkResampleImage](@ref MitkResampleImagePage). This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkResampleMask -i <mask> -o <output> -x <spacing> -y <spacing> -z <spacing> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | File | Input mask. Expected pixel type is unsigned short. |
| `--output` | `-o` | File | Output mask. |
| `--spacing-x` | `-x` | Float | New spacing in x direction (mm). A value of 0 or less keeps the original spacing. |
| `--spacing-y` | `-y` | Float | New spacing in y direction (mm). A value of 0 or less keeps the original spacing. |
| `--spacing-z` | `-z` | Float | New spacing in z direction (mm). A value of 0 or less keeps the original spacing. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--mapping-type` | `-mapping` | Int | 0 | Grid alignment: 0 origin aligned, 1 center aligned, 2 same size. |
| `--interpolator-type` | `-interpolator` | Int | 0 | Interpolation: 0 linear, 1 B-spline (order 3), 2 nearest neighbour, 3 windowed sinc (Hamming), 4 windowed sinc (Welch). |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

The grid alignment options are the same as for [MitkResampleImage](@ref MitkResampleImagePage): origin aligned keeps the origin, center aligned keeps the image center, same size keeps the physical extent. Out-of-range values fall back to origin aligned and linear.

The first object in the input file is used. If the file cannot be read or does not contain an image, the app reports the error and exits with code 1.

## Examples

### Resample a mask to match a 1 mm isotropic image

```bash
MitkResampleMask -i tumor.nrrd -o tumor_1mm.nrrd -x 1 -y 1 -z 1
```

Linear interpolation followed by thresholding at 0.5 yields a smooth binary boundary.

### Nearest neighbour resampling

```bash
MitkResampleMask -i tumor.nrrd -o tumor_2mm.nrrd -x 2 -y 2 -z 2 -interpolator 2
```
