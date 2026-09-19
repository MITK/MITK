# MitkImageTypeConverter {#MitkImageTypeConverterPage}

[TOC]

## Overview

MitkImageTypeConverter converts the scalar pixel type of a 2D or 3D image to a given type, for example from `float` to `unsigned short`. It loads the image, casts every pixel to the requested type, and writes the result to the output path. The output format is determined by the file extension of the output path.

Use it to standardize the pixel type of images before processing them with tools that require a specific type. To convert between file formats without changing the pixel type, use [MitkFileConverter](@ref MITKFileConverterPage).

## Usage

```bash
MitkImageTypeConverter -i <input> -o <output> -t <type>
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Path to the input image. |
| `--output` | `-o` | File | Path to the output image. The extension selects the output format. |
| `--type` | `-t` | String | Target scalar pixel type. One of `int`, `uint`, `short`, `ushort`, `char`, `uchar`, `float`, `double`, or `none` (see Details). |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Supported target types

| Value | Resulting pixel type |
|-------|----------------------|
| `int` | signed 32-bit integer |
| `uint` | unsigned 32-bit integer |
| `short` | signed 16-bit integer |
| `ushort` | unsigned 16-bit integer |
| `char` | signed 8-bit integer |
| `uchar` | unsigned 8-bit integer |
| `float` | single-precision floating point |
| `double` | double-precision floating point |
| `none` | no conversion is performed and no output file is written |

Any other value is rejected with an error message and exit code 1.

### Casting behaviour

Pixel values are cast one by one (as with `itk::CastImageFilter`). There is no rescaling and no clamping, so values outside the range of the target type overflow or are truncated, and fractional values lose their decimals when converting to an integer type. Make sure the value range of the input fits the target type.

### Dimensions

Only 2D and 3D images are converted. For any other dimension (e.g. a 3D+t image) the app prints `This tool doesn't support a dimension of <n>.`, writes no output, and exits with 1. To convert a dynamic image, split it first with [MitkSplit4Dto3DImages](@ref MitkSplit4Dto3DImagePage), convert the time steps, and fuse them again with [MitkFuse3Dto4DImage](@ref MitkFuse3Dto4DImagePage).

### Exit code

The app exits with 0 after writing the output and for `none`, where no file is written on purpose. It exits with 1 if required arguments are missing, the type is unknown, or the image dimension is not supported.

## Examples

### Convert a CT image to unsigned short

```bash
MitkImageTypeConverter -i ct_float.nrrd -o ct_ushort.nrrd -t ushort
```

Every pixel of `ct_float.nrrd` is cast to `unsigned short` and the result is written to `ct_ushort.nrrd`. Negative values (e.g. Hounsfield units below 0) cannot be represented in the target type and the result for such pixels is not defined; shift or clamp the values beforehand if needed.

### Convert a binary mask to unsigned char

```bash
MitkImageTypeConverter -i mask.nii.gz -o mask_uchar.nii.gz -t uchar
```

A mask stored with a wider integer type is reduced to one byte per pixel. The output is written as compressed NIfTI because of the `.nii.gz` extension.

### Convert to float for downstream processing

```bash
MitkImageTypeConverter -i mr.mhd -o mr_float.mhd -t float
```

The image is converted to single-precision floating point, which is what many filters expect as input.
