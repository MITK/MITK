# MitkMaskRangeBasedFiltering {#MitkMaskRangeBasedFilteringPage}

[TOC]

## Overview

MitkMaskRangeBasedFiltering cleans a mask with the help of the underlying image: every mask voxel whose image intensity lies below a lower limit or above an upper limit is removed from the mask. Either limit, or both, can be given. This is useful to strip air or bone from an organ mask, or to restrict a mask to a known intensity window.

For a statistical variant that derives the limits from the masked intensities see [MitkMaskOutlierFiltering](@ref MitkMaskOutlierFilteringPage). This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkMaskRangeBasedFiltering -i <image> -m <mask> -o <output mask> [--lower-limit <value>] [--upper-limit <value>]
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
| `--lower-limit` | `-lL` | Float | | Mask voxels covering image intensities below this value are set to 0. |
| `--upper-limit` | `-ul` | Float | | Mask voxels covering image intensities above this value are set to 0. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

At least one limit must be given. Without a limit the app prints a message and exits with code 0 without writing an output. Note the short name of the lower limit, `-lL` with a capital L.

Image and mask must have the same geometry. The first object of each input file is used; if it is not an image, the app prints a message and exits with code 0.

## Examples

### Remove air from a lung mask

```bash
MitkMaskRangeBasedFiltering -i ct.nrrd -m lung.nrrd -o lung_clean.nrrd --lower-limit -950
```

### Keep only soft tissue

```bash
MitkMaskRangeBasedFiltering -i ct.nrrd -m body.nrrd -o soft_tissue.nrrd --lower-limit -100 --upper-limit 200
```
