# MitkLaplacianOfGaussian {#MitkLaplacianOfGaussianPage}

[TOC]

## Overview

MitkLaplacianOfGaussian smooths an image with a Gaussian kernel of the given sigma and computes the Laplacian of the result. The filter responds to edges and blobs at the scale defined by sigma and is a common feature image in radiomics pipelines, for example as input to [MitkCLGlobalImageFeatures](@ref MitkCLGlobalImageFeaturesPage).

This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkLaplacianOfGaussian -i <image> -o <output> -s <sigma> [--as-double]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | File | Input image. |
| `--output` | `-o` | File | Output image. |
| `--sigma` | `-s` | Float | Standard deviation of the Gaussian kernel in physical units (mm). |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--as-double` | `-double` | Flag | off | Write the result with pixel type double instead of the input pixel type. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

The Laplacian of Gaussian is signed and usually fractional. Without `--as-double` the result is cast to the pixel type of the input, which truncates values and clips negative values on unsigned images. Use `--as-double` unless you know the input type can hold the result.

The first object in the input file is used. If the file cannot be read or does not contain an image, the app reports the error and exits with code 1.

## Examples

### LoG at 2 mm scale

```bash
MitkLaplacianOfGaussian -i ct.nrrd -o ct_log2.nrrd -s 2 --as-double
```

### Several scales for a feature bank

```bash
MitkLaplacianOfGaussian -i ct.nrrd -o ct_log1.nrrd -s 1 --as-double
MitkLaplacianOfGaussian -i ct.nrrd -o ct_log3.nrrd -s 3 --as-double
MitkLaplacianOfGaussian -i ct.nrrd -o ct_log5.nrrd -s 5 --as-double
```
