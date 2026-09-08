# MitkCLMRNormalization {#MitkCLMRNormalizationPage}

[TOC]

## Overview

MitkCLMRNormalization rescales the intensities of an MR image linearly so that reference tissue regions end up at defined values. The reference regions are given as mask images. In the single-region modes the mean, median or mode of the region covered by `--mask0` is shifted to zero and the intensities are divided by the standard deviation of that region. In the two-region modes a statistic of the region covered by `--mask0` is mapped to 0 and the same statistic of the region covered by `--mask1` is mapped to 1.

The input is an MR image and one or two mask images of the same dimension. The output is an image with the same geometry and pixel type as the input. Since the normalized values are typically small floating point numbers, integer input images should be converted to float with `--float` (or beforehand with [MitkImageTypeConverter](@ref MitkImageTypeConverterPage)); otherwise the normalized values are truncated to the integer type of the input.

Related apps: [MitkCLN4](@ref MitkCLN4Page) removes the bias field from MR images, typically before normalization, and [MitkCLGlobalImageFeatures](@ref MitkCLGlobalImageFeaturesPage) computes features on the normalized image. This app is built with the CMake option `BUILD_ClassificationCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkCLMRNormalization -i <image> -mode <1-6> -m0 <mask0> [-m1 <mask1>] -o <output> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | Image | Path to the input MR image. The help text calls it a VTK polydata; it is an image. |
| `--mode` | `-mode` | Int | Normalization mode 1 to 6, see Details. The argument is declared as Image type in the code, so the help text does not show it as an integer; the value is converted with `std::stoi`. |
| `--mask0` | `-m0` | Image | Mask of the first reference region. Voxels with the value 1 belong to the region. |
| `--output` | `-o` | File | Path of the normalized output image. The help text speaks of an appended statistic; the app writes an image. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--help` | `-h` | Flag | | Not a declared argument. The help text is printed by the argument parser whenever a required argument is missing, so calling the app with `-h` alone shows the help; the app then exits with code 1. |
| `--mask1` | `-m1` | Image | | Mask of the second reference region. Required for modes 4 to 6, ignored for modes 1 to 3. Voxels with the value 1 belong to the region. |
| `--ignore-outlier` | `-outlier` | Flag | | Restrict the statistics of the `--mask0` region to the 2nd to 98th percentile and clamp all image intensities to that range before normalization. Only used in modes 1 to 3. |
| `--value` | `-v` | Float | `0` | Offset added to the center statistic of the `--mask0` region before it is subtracted, see Details. Only used in modes 1 to 3. |
| `--width` | `-w` | Float | | Intended as a scaling factor for the standard deviation, but the code passes the value to the same setter as `--value`. Giving `--width` therefore overwrites `--value`; the actual width stays 1. Only used in modes 1 to 3. |
| `--float` | `-float` | Flag | | Cast the input image to float before processing so that the output is a float image. |

## Details

### Normalization modes

| Mode | Regions | Statistic | Result |
|------|---------|-----------|--------|
| 1 | `--mask0` | mean | `(value - (mean + v)) / stddev` |
| 2 | `--mask0` | median | `(value - (median + v)) / stddev` |
| 3 | `--mask0` | mode (most frequent bin of a 256-bin histogram) | `(value - (mode + v)) / stddev` |
| 4 | `--mask0`, `--mask1` | mean of each region | `(value - min(mean0, mean1)) / (max(mean0, mean1) - min(mean0, mean1))` |
| 5 | `--mask0`, `--mask1` | median of each region | analogous with medians |
| 6 | `--mask0`, `--mask1` | mode of each region | analogous with modes |

`v` is the value of `--value` (0 by default) and `stddev` the standard deviation of the intensities inside the `--mask0` region. Note that a positive `--value` moves the region center to a negative output value: the center is mapped to `-v / stddev`, not to `v` as the help text suggests.

Any other mode number produces no output image and the app fails with an unhandled exception when it tries to save.

### Statistics and masks

Statistics are computed with an ITK label statistics filter over the voxels whose mask value is exactly 1 (masks are cast to integer). The histogram used for the median and the mode has 256 bins over the intensity range of the whole image, or, with `--ignore-outlier`, over the 2nd to 98th percentile range of the `--mask0` region computed from a 2048-bin histogram. With `--ignore-outlier` every intensity outside that percentile range is clamped to the range before the linear transformation is applied, in the whole image, not only inside the mask.

If the denominator of the transformation is smaller than 0.0001 (a constant region, or two regions with identical statistics), the filter returns without writing any voxel; the output image is then saved with uninitialized content.

### Pixel type and time steps

The output image is initialized with the pixel type and geometry of the input. The transformation is applied voxel by voxel in the input type, so integer input types truncate the result; use `--float` to obtain floating point output. The masks are cast to the dimension of the input image and the whole image buffer is processed as one block, time steps are not treated separately. Use 3D images and 3D masks.

### Exit behaviour

The app returns 0 after writing the output. Missing required arguments print the help text and return 1. Errors such as unreadable files, a missing `--mask1` in modes 4 to 6, or an invalid mode raise exceptions that are not caught; the process terminates with the platform-specific code for an unhandled exception.

## Examples

### Normalize to the median of white matter

```bash
MitkCLMRNormalization -i t1.nrrd -mode 2 -m0 white_matter.nrrd -o t1_normalized.nrrd -float
```

Casts `t1.nrrd` to float, subtracts the median intensity of the white matter region and divides by its standard deviation. White matter ends up around 0 with unit spread.

### Mean-based normalization with outlier clamping

```bash
MitkCLMRNormalization -i t2.nrrd -mode 1 -m0 brain_mask.nrrd -o t2_normalized.nrrd -outlier -float
```

Computes mean and standard deviation of the brain region from the intensities between the 2nd and 98th percentile, clamps the whole image to that range and applies the linear transformation.

### Two-region normalization between CSF and white matter

```bash
MitkCLMRNormalization -i flair.nrrd -mode 5 -m0 csf.nrrd -m1 white_matter.nrrd -o flair_normalized.nrrd -float
```

Maps the lower of the two region medians to 0 and the higher one to 1; all other intensities are scaled linearly in between and beyond.
