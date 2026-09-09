# MitkSingleImageArithmetic {#MitkSingleImageArithmeticPage}

[TOC]

## Overview

MitkSingleImageArithmetic applies unary mathematical functions to every voxel of an image, for example the square root, the logarithm or a trigonometric function. Several functions can be requested in one call; they are applied one after another in a fixed order.

For operations between two images see [MitkTwoImageArithmetic](@ref MitkTwoImageArithmeticPage), for operations between an image and a constant see [MitkImageAndValueArithmetic](@ref MitkImageAndValueArithmeticPage). This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkSingleImageArithmetic -i <input> -o <output> [--as-double] [operation flags]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Input image. |
| `--output` | `-o` | File | Output image. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--as-double` | `-double` | Flag | off | Write the result with pixel type double instead of the input pixel type. |
| `--tan` | `-tan` | Flag | off | Tangent. |
| `--atan` | `-atan` | Flag | off | Arc tangent. |
| `--cos` | `-cos` | Flag | off | Cosine. |
| `--acos` | `-acos` | Flag | off | Arc cosine. |
| `--sin` | `-sin` | Flag | off | Sine. |
| `--asin` | `-asin` | Flag | off | Arc sine. |
| `--square` | `-square` | Flag | off | Square. |
| `--sqrt` | `-sqrt` | Flag | off | Square root. |
| `--abs` | `-abs` | Flag | off | Absolute value. |
| `--exp` | `-exp` | Flag | off | Exponential function. |
| `--expneg` | `-expneg` | Flag | off | Exponential function of the negated value, exp(-x). |
| `--log10` | `-log10` | Flag | off | Logarithm to base 10. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

If more than one operation flag is given, the operations are applied in the order of the table above (tan, atan, cos, acos, sin, asin, square, sqrt, abs, exp, expneg, log10), each on the result of the previous one. Without any operation flag the input is written unchanged.

Without `--as-double` the result keeps the pixel type of the input, so fractional results are truncated for integer images. Use `--as-double` whenever the function produces non-integer values.

The first object in the input file is used. If the file cannot be read or does not contain an image, the app reports the error and exits with code 1.

## Examples

### Square root of a CT image as double

```bash
MitkSingleImageArithmetic -i ct.nrrd -o ct_sqrt.nrrd --sqrt --as-double
```

### Logarithm of a PET image

```bash
MitkSingleImageArithmetic -i pet.nrrd -o pet_log10.nrrd --log10 --as-double
```

### Chain two operations

```bash
MitkSingleImageArithmetic -i map.nrrd -o map_abs_sqrt.nrrd --abs --sqrt --as-double
```

Computes sqrt(abs(x)) for every voxel, because abs precedes sqrt in the fixed order.
