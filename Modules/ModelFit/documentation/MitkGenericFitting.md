# MitkGenericFitting {#MitkGenericFittingPage}

[TOC]

## Overview

MitkGenericFitting fits a model function to the intensity curve over time of
a dynamic (3D+t) image and writes one image per fitted quantity. The model is
either a linear function or a user-defined formula that is parsed at runtime.
Fitting is done voxel by voxel (optionally restricted to a mask) or once for
the mean curve of a region of interest.

The fit uses a Levenberg-Marquardt optimizer. Besides the model parameters the
application stores derived parameters, the fit criterion, an evaluation value
and debug maps of the optimizer. Related applications for dynamic data are
[MitkCurveDescriptor](@ref MitkCurveDescriptorPage) (descriptive curve
parameters without a model) and [MitkMRPerfusion](@ref MitkMRPerfusionPage)
(pharmacokinetic models).

## Usage

```bash
MitkGenericFitting -i <input> -o <output template> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Dynamic (3D+t) image whose time curves are fitted. |
| `--output` | `-o` | File | Template path for the result images. Directory, base name and extension are taken from it; a suffix per result is appended to the base name. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--function` | `-f` | String | `Linear` | Model to fit. `Linear` selects the linear model, `Generic-<N>` the generic formula model with N free parameters (1 to 10). |
| `--formular` | `-y` | String | | Formula of the generic model. Required for `Generic-<N>`. |
| `--mask` | `-m` | File | | Mask image that defines the voxels to fit. Must have the same geometry as the input. Required for `--roibased`. |
| `--roibased` | `-r` | Flag | | Fit the mean curve of the mask region once instead of every voxel. |
| `--verbose` | `-v` | Flag | | Accepted for compatibility; has no effect. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Models

`--function Linear` fits `y(x) = slope * x + y-intercept`. Its parameters are
named `slope` and `y-intercept`; the derived parameter `x-intercept`
(`-y-intercept / slope`) is stored as well.

`--function Generic-<N>` selects the generic formula model with N free
parameters, whose function is the string given with `--formular`. The
independent variable is `x`, the time of the respective time step in seconds
(taken from the time geometry of the input). The free parameters are named
`a`, `b`, `c`, ... up to the N-th letter; N must be between 1 and 10. Any other
value of `--function`, an N outside that range, or a missing formula is
rejected before any data is loaded.

The formula parser understands:

- numeric literals with optional decimal point and exponent (`1.5`, `.5`,
  `1e-3`);
- sums, differences, products and divisions (`a + 3`, `4 - x`, `2 * x`,
  `a / 3`);
- algebraic signs (`+5`, `-5`);
- exponentiation (`x ^ 2`);
- parentheses (`3 * (x + 2)`);
- the unary functions `abs`, `exp`, `sin`, `cos`, `tan`, `sind`, `cosd`,
  `tand` (the `d` variants take degrees), `fresnelS` and `fresnelC`.

Whitespace is only allowed between tokens. All model parameters start at an
initial value of `1.0`.

### Fitting modes

Without `--roibased` the model is fitted to the curve of every voxel. If a
mask is given, only voxels inside the mask are fitted; the other voxels of the
result images stay `0`.

With `--roibased` the mean curve over all mask voxels is computed first and
the model is fitted once to that curve. The fitted values are then written
into every voxel of the mask; a mask is mandatory in this mode and the
application fails without one.

### Output files

Every result is stored as a separate image. The file name is
`<root>_<name>.<ext>`, where `<root>` and `<ext>` come from `--output` and
`<name>` is the result name with the characters `\ / : ? " < > | % *` and
spaces replaced by `_`. The results are:

| Category | Names |
|----------|-------|
| Model parameters | `slope`, `y-intercept` (linear) or `a`, `b`, ... (generic). |
| Derived parameters | `x-intercept` (linear only). |
| Fit criterion | `sum_diff^2`, the sum of squared differences between model and data. |
| Evaluation | `Chi^2`, the normalized sum of squared differences. |
| Optimizer debug maps | `optimization_time`, `nr_of_iterations`, `stop_condition`. |

The console lists every stored file. The input and the mask are loaded with
the "MITK DICOM Reader v2 (autoselect)" reader preferred for DICOM data.

## Examples

### Voxel-wise linear fit

```bash
MitkGenericFitting -i dce_series.nrrd -o results/linear.nrrd
```

Fits the linear model to every voxel and writes `results/linear_slope.nrrd`,
`results/linear_y-intercept.nrrd`, `results/linear_x-intercept.nrrd`,
`results/linear_sum_diff^2.nrrd`, `results/linear_Chi^2.nrrd` and the three
optimizer debug maps.

### Voxel-wise fit of a user-defined formula inside a mask

```bash
MitkGenericFitting -i dce_series.nrrd -m tumor_mask.nrrd -o results/washout.nrrd -f Generic-2 -y "a * exp(-b * x)"
```

Fits an exponential decay with the two free parameters `a` and `b` to every
voxel inside `tumor_mask.nrrd` and writes `results/washout_a.nrrd` and
`results/washout_b.nrrd` together with the criterion, evaluation and debug
images. Voxels outside the mask are `0`.

### ROI-based linear fit

```bash
MitkGenericFitting -i dce_series.nrrd -m tumor_mask.nrrd -o results/roi_linear.nrrd -r
```

Averages the curves of all voxels in `tumor_mask.nrrd`, fits the linear model
once and writes the fitted values into the mask region of each result image.
