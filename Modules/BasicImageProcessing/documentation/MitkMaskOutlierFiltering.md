# MitkMaskOutlierFiltering {#MitkMaskOutlierFilteringPage}

[TOC]

## Overview

MitkMaskOutlierFiltering removes statistical outliers from a mask. It computes the mean and the standard deviation of the image intensities inside the mask and removes every mask voxel whose intensity lies outside the range of mean plus or minus three standard deviations. This is a common preprocessing step before feature extraction, for example with [MitkCLGlobalImageFeatures](@ref MitkCLGlobalImageFeaturesPage).

To specify the limits yourself use [MitkMaskRangeBasedFiltering](@ref MitkMaskRangeBasedFilteringPage). This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkMaskOutlierFiltering -i <image> -m <mask> -o <output mask>
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | File | Grey value image that provides the intensities. |
| `--mask` | `-m` | File | Mask to clean. Expected pixel type is unsigned short. |
| `--output` | `-o` | File | Output mask. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

Image and mask must have the same geometry. The first object of each input file is used; if a file cannot be read or does not contain an image, the app reports the error and exits with code 1.

## Examples

### Clean a tumor segmentation before radiomics

```bash
MitkMaskOutlierFiltering -i t1.nrrd -m tumor.nrrd -o tumor_clean.nrrd
```

Voxels of the tumor mask whose T1 intensity deviates by more than three standard deviations from the mean tumor intensity are removed.
