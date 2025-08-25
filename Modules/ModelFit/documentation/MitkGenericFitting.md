# MitkGenericFitting User Guide {#MitkGenericFittingPage}

[TOC]

## Overview

`MitkGenericFitting` is a command-line tool for fitting model functions to time-resolved imaging data (3D+t). It supports voxel-wise or region-based fitting using a mask and produces parameter images as output.

Supported image types for input include `.nrrd`, `.nii`, and DICOM files.

## Supported Models

### Linear Model
- Model: `y = a + b*t`
- Specify with: `--function Linear`

### Generic Model (2 parameters)
- User-defined model formula using two free parameters (`a`, `b`)
- Example: `y = a * exp(-b*x)`
- The parser is able to recognize:
  - sums, differences, products and divisions (a + b, 4 - 3, 2 * x, 9 / 3)
  - algebraic signs ( +5, -5)
  - exponentiation ( 2 ^ 4 )
  - parentheses (3 * (4 + 2))
  - following unary functions: abs, exp, sin, cos, tan, sind (sine in degrees), cosd (cosine in degrees), tand (tangent in degrees)
  - variables (x, a, b, ... j)

**Remarks**
- The variable "x" is reserved. It is the signal position / timepoint.
- The current version supports 2 model parameter (a, b)
---

## Usage

```bash
MitkGenericFitting -i <inputImage> -o <outputTemplate> -f <functionType> [options]
```

### Required Arguments

| Argument        | Short | Type   | Description |
|----------------|-------|--------|-------------|
| `--input`       | `-i`  | file   | Input dynamic image (3D+t). |
| `--output`      | `-o`  | file   | Output template file path for resulting parameter images. File extensions determine output format. |
| `--function`      | `-f`  | string | Model type: `"Linear"` or `"Generic-2"` for generic 2 parameter model. Default is `Linear`. |


### Optional Arguments

| Argument        | Short | Type   | Description |
|-----------------|-------|--------|-------------|
| `--formular`      | `-y`  | string | Model formula string (required for generic models). |
| `--mask`         | `-m`  | file   | Binary mask image defining ROI. Required for ROI-based fitting. |
| `--roibased`     | `-r`  | flag   | Use ROI-based fitting (fit mean signal in mask). |
| `--verbose`      | `-v`  | flag   | Enable verbose output. |
| `--help`         | `-h`  | flag   | Show help text. |


## Output

For each fitted model parameter, a separate image will be created. The output filenames will be based on the `--output` template plus a suffix per parameter (e.g., `_Param_0.nrrd`, `_Param_1.nrrd`).  
A fitting summary and evaluation metric image (e.g., `Chi^2`) will also be generated.

## Fitting Modes

### Pixel-based Fitting (Default)

- The model is fitted to each voxel individually.
- Mask is optional (if provided, fitting is constrained to masked region).

### ROI-based Fitting

- The average time-intensity curve within the mask is computed.
- The model is fitted to the ROI curve.
- Output will be constant-valued images reflecting the ROI fit result.


## Examples

### 1. Linear Model Fit (Pixel-based)

```bash
MitkGenericFitting -i dyn.nrrd -o result/linFit.nrrd -f Linear
```

**Description**:  
Performs voxel-wise fitting of a linear model on `dyn.nrrd`.

**Naming Strategy**:
- Output parameter images: `linFit_y-intercept.nrrd` (intercept), `linFit_slope.nrrd` (slope)

**Generated Values**:
- Each voxel gets individual values for intercept and slope.

---

### 2. Exponential Decay (Generic, Pixel-based)

```bash
MitkGenericFitting -i dyn.nrrd -o result/expFit.nrrd -f Generic-2 -y "a*exp(-b*x)"
```

**Description**:  
Fits an exponential decay model to each voxel.

**Naming Strategy**:
- Output files: `expFit_a.nrrd` (a), `expFit_b.nrrd` (b)

**Generated Values**:
- Each voxel gets a fitted p0 and p1 value for the exponential model.

---

### 3. ROI-based Linear Fit

```bash
MitkGenericFitting -i dyn.nrrd -o result/roiLinFit.nrrd -f Linear -m mask.nrrd -r
```

**Description**:  
Fits a linear model to the mean curve inside the mask.

**Naming Strategy**:
- Output parameter images: `roiLinFit_y-intercept.nrrd` (intercept), `roiLinFit_slope.nrrd` (slope)

**Generated Values**:
- Each output image has constant values representing ROI fit result.

---