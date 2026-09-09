# MitkTwoImageArithmetic {#MitkTwoImageArithmeticPage}

[TOC]

## Overview

MitkTwoImageArithmetic combines two images voxel by voxel with a basic arithmetic operation: addition, subtraction, multiplication or division. Both images must have the same dimensions.

For a single image see [MitkSingleImageArithmetic](@ref MitkSingleImageArithmeticPage), for an image and a constant see [MitkImageAndValueArithmetic](@ref MitkImageAndValueArithmeticPage). This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkTwoImageArithmetic -i1 <left image> -i2 <right image> -o <output> [--as-double] (--add | --subtract | --multiply | --divide)
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input-left` | `-i1` | File | Left operand image. |
| `--input-right` | `-i2` | File | Right operand image. |
| `--output` | `-o` | File | Output image. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--as-double` | `-double` | Flag | off | Write the result with pixel type double instead of the pixel type of the left image. |
| `--add` | `-add` | Flag | off | left + right. |
| `--subtract` | `-sub` | Flag | off | left - right. |
| `--multiply` | `-multi` | Flag | off | left * right. |
| `--divide` | `-div` | Flag | off | left / right. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

If more than one operation flag is given, the operations are applied in the order add, subtract, multiply, divide, each using the result of the previous one as left operand and the right image as right operand. Without any operation flag the left image is written unchanged.

Without `--as-double` the result keeps the pixel type of the left image. Use `--as-double` for divisions and for subtractions that can become negative on unsigned images.

The first object of each input file is used. If a file cannot be read or does not contain an image, the app reports the error and exits with code 1.

## Examples

### Difference of two time points

```bash
MitkTwoImageArithmetic -i1 followup.nrrd -i2 baseline.nrrd -o difference.nrrd --subtract --as-double
```

### Ratio image

```bash
MitkTwoImageArithmetic -i1 pet.nrrd -i2 reference.nrrd -o ratio.nrrd --divide --as-double
```

### Apply a mask by multiplication

```bash
MitkTwoImageArithmetic -i1 ct.nrrd -i2 mask.nrrd -o ct_masked.nrrd --multiply
```

Voxels outside the mask (value 0) become 0, voxels inside (value 1) keep their intensity.
