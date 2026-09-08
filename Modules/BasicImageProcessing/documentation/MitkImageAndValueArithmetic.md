# MitkImageAndValueArithmetic {#MitkImageAndValueArithmeticPage}

[TOC]

## Overview

MitkImageAndValueArithmetic combines every voxel of an image with a constant value: addition, subtraction, multiplication or division. By default the image is the left operand (image - value); with `--image-right` the value becomes the left operand (value - image).

For two images see [MitkTwoImageArithmetic](@ref MitkTwoImageArithmeticPage), for unary functions see [MitkSingleImageArithmetic](@ref MitkSingleImageArithmeticPage). This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkImageAndValueArithmetic -i <image> -v <value> -o <output> [--as-double] [--image-right] (--add | --subtract | --multiply | --divide)
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | File | Input image. |
| `--value` | `-v` | Float | Constant value. |
| `--output` | `-o` | File | Output image. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--as-double` | `-double` | Flag | off | Write the result with pixel type double instead of the input pixel type. |
| `--image-right` | `-right` | Flag | off | Use the value as left operand and the image as right operand (value - image, value / image). |
| `--add` | `-add` | Flag | off | Addition. |
| `--subtract` | `-sub` | Flag | off | Subtraction. |
| `--multiply` | `-multi` | Flag | off | Multiplication. |
| `--divide` | `-div` | Flag | off | Division. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

If more than one operation flag is given, the operations are applied in the order add, subtract, multiply, divide, each on the result of the previous one. Without any operation flag the input is written unchanged.

Without `--as-double` the result keeps the pixel type of the input, so a multiplication by 0.5 on an integer image truncates. The value is parsed as single precision float.

The first object in the input file is used. If it is not an image, the app prints a message and exits with code 0 without writing an output.

## Examples

### Scale intensities

```bash
MitkImageAndValueArithmetic -i ct.nrrd -v 0.001 -o ct_scaled.nrrd --multiply --as-double
```

### Shift Hounsfield units into a positive range

```bash
MitkImageAndValueArithmetic -i ct.nrrd -v 1024 -o ct_shifted.nrrd --add
```

### Invert a probability map

```bash
MitkImageAndValueArithmetic -i prob.nrrd -v 1 -o inverted.nrrd --subtract --image-right --as-double
```

Computes 1 - prob for every voxel because `--image-right` puts the value on the left side.
