# MitkCLN4 {#MitkCLN4Page}

[TOC]

## Overview

MitkCLN4 corrects the low-frequency intensity inhomogeneity (bias field) of an MR image with the N4 bias field correction algorithm of ITK (`itk::N4BiasFieldCorrectionImageFilter`). The input is a 3D image and a 3D mask that marks the voxels used for estimating the bias field; the output is the corrected image as a float image with the geometry of the input. The B-spline fitting of the algorithm can be tuned with the optional parameters.

Related apps: [MitkCLMRNormalization](@ref MitkCLMRNormalizationPage) normalizes the intensities of the corrected image, and [MitkCLGlobalImageFeatures](@ref MitkCLGlobalImageFeaturesPage) computes features on it. This app is built with the CMake option `BUILD_ClassificationCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkCLN4 -i <image> -m <mask> -o <output> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Path to the input image. |
| `--mask` | `-m` | File | Path to the mask image. All voxels with a value other than 0 are used for the bias field estimation. |
| `--output` | `-o` | File | Path of the corrected output image. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--help` | `-h` | Flag | | Show the help text and exit. |
| `--number-of-controllpoints` | `-noc` | Int | `4` | Number of B-spline control points per dimension for the initial fitting level. |
| `--number-of-fitting-levels` | `-nofl` | Int | `1` | Number of fitting levels of the multi-scale approach; the B-spline mesh resolution doubles with every level. |
| `--number-of-histogram-bins` | `-nohb` | Int | `200` | Number of bins of the log intensity histogram. |
| `--spline-order` | `-so` | Int | `3` | Order of the B-spline used for the bias field estimate. |
| `--winer-filter-noise` | `-wfn` | Float | `0.01` | Noise estimate of the Wiener filter used for sharpening the histogram. |
| `--number-of-maximum-iterations` | `-nomi` | Int | `50` | Maximum number of iterations. The value is applied to every fitting level. |

## Details

### Processing

The input image is cast to a 3D float image and the mask to a 3D unsigned char image. Both must be 3D; 2D or 3D+t data fails with an exception when it is cast. Every voxel of the mask that is not 0 is included in the estimation (the ITK filter is left in its default of not restricting to a particular mask label). The corrected image is written as a float image with the geometry of the input; the estimated bias field itself is not saved.

The defaults in the table are the defaults of the ITK filter, which is used unchanged for every parameter that is not given.

### Argument handling

If a required argument is missing, the parser prints the help text and the app returns 1. `--help` in combination with all required arguments prints the help text and exits with code 0.

### Exit behaviour

The app returns 0 after writing the output. Errors during processing (unreadable files, dimension mismatch, unwritable output) are reported on the console and the app returns 1.

## Examples

### Correct a T1 image with the default settings

```bash
MitkCLN4 -i t1.nrrd -m brain_mask.nrrd -o t1_corrected.nrrd
```

Estimates the bias field from the voxels inside `brain_mask.nrrd` with one fitting level, four control points per dimension, a spline order of 3 and at most 50 iterations, and writes the corrected float image.

### Multi-scale fitting with more iterations

```bash
MitkCLN4 -i t1.nrrd -m brain_mask.nrrd -o t1_corrected.nrrd -nofl 4 -nomi 100
```

Runs four fitting levels, doubling the B-spline mesh resolution with every level, and allows up to 100 iterations on each level.

### Adjust the histogram sharpening and spline order

```bash
MitkCLN4 -i t2.nrrd -m head_mask.nrrd -o t2_corrected.nrrd -so 2 -wfn 0.05
```

Uses a quadratic B-spline and a larger noise estimate for the Wiener deconvolution of the intensity histogram.
