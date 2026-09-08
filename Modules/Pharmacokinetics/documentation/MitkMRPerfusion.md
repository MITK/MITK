# MitkMRPerfusion {#MitkMRPerfusionPage}

[TOC]

## Overview

`MitkMRPerfusion` fits pharmacokinetic and descriptive models to a dynamic contrast-enhanced (DCE) MR image and writes one 3D parameter image per fitted, derived and evaluation parameter. The available models are the extended Tofts model, the two compartment exchange model, the descriptive pharmacokinetic Brix model, and the two and three step linear models. Fitting is done either voxel by voxel (pixel based) or once for the mean curve of a region of interest (ROI based).

The input is a 3D+t image. Important: the app assumes that the input image and the AIF image already hold contrast agent concentration values. If your data are MR signal intensities, convert them first with [MitkMRSignal2Concentration](@ref MitkMRSignal2ConcentrationPage). For model-free curve descriptors (AUC, time to peak, ...) use [MitkCurveDescriptor](@ref MitkCurveDescriptorPage); for fitting arbitrary user-defined formulas use [MitkGenericFitting](@ref MitkGenericFittingPage). If the dynamic series is only available as separate 3D volumes, [MitkFuse3Dto4DImage](@ref MitkFuse3Dto4DImagePage) can assemble them into a 3D+t image first.

This app is built with the CMake option `BUILD_PerfusionCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkMRPerfusion -i <input> -o <output> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Input 3D+t concentration image. |
| `--output` | `-o` | File | Output file template. Directory and extension determine where and in which format the parameter images are written; the file name without extension is used as root name, and a suffix is added for each parameter. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--model` | `-l` | String | `tofts` | Model to fit. One of `descriptive` (descriptive pharmacokinetic Brix model), `2SL` (two step linear model), `3SL` (three step linear model), `tofts` (extended Tofts model) or `2CX` (two compartment exchange model). Case-sensitive. |
| `--injectiontime` | `-j` | Float | `0` | Injection time of the bolus in minutes. Only used by the `descriptive` model, which requires a non-zero value. |
| `--aifmask` | `-n` | File | | Mask that defines the voxels used to derive the arterial input function (AIF). Required for `tofts` and `2CX`. Must have the same geometry as the AIF image. |
| `--aifimage` | `-a` | File | | 3D+t image from which the AIF is extracted. If not given, the AIF is extracted from the input image. |
| `--hematocrit` | `-h` | Float | `0.45` | Hematocrit level used to convert the AIF from blood to plasma concentration. Only used for AIF based models. |
| `--mask` | `-m` | File | | Mask that defines the voxels to fit. Optional for pixel based fitting, required for ROI based fitting. Must have the same geometry as the input image. |
| `--roibased` | `-r` | Flag | | Fit the mean curve of the mask region once instead of fitting each voxel. Requires `--mask`. |
| `--preview` | `-p` | Flag | | Load the inputs, set up the fit and print the output file paths the app would produce, without fitting. |
| `--verbose` | `-v` | Flag | | Accepted, but currently has no effect on the output. |

There is no working `--help` argument, see "Help text" below.

## Details

### Model selection

`--model` is compared case-sensitively against the five names listed above; `tofts` is used when the argument is omitted. The models fall into two groups:

- AIF based models (`tofts`, `2CX`) need an arterial input function. `--aifmask` is mandatory for them, `--aifimage` and `--hematocrit` are evaluated.
- Models without AIF (`descriptive`, `2SL`, `3SL`) silently ignore `--aifmask`, `--aifimage` and `--hematocrit`.

An unknown model name is not rejected up front. It is treated like an AIF based model, so without `--aifmask` the app fails with "Chosen model needs an AIF"; with an AIF mask it fails later with "Model flag is unknown" followed by "Could not initialize fitting job". Both cases exit with code 1.

The time grid of all models is extracted from the time geometry of the input image and used in seconds. Concentration values are expected in mM for the AIF based models.

### Arterial input function

For `tofts` and `2CX` the AIF is computed as follows:

1. The AIF source image is `--aifimage` if given, otherwise the input image.
2. If the AIF mask has more than one time step, only its first time step is used.
3. For every time step the mean value of all voxels with a non-zero mask value is computed.
4. Each mean value is divided by `(1 - hematocrit)` to convert blood to plasma concentration. The hematocrit level is printed to the console.
5. The AIF time grid is taken from the time geometry of the AIF source image.

The resulting AIF values are stored in the fit information that is attached to the result images.

### Masks and fitting styles

- If `--mask` has more than one time step, only its first time step is used.
- Pixel based fitting (default): every voxel of the input image is fitted independently. If a mask is given, only voxels with a non-zero mask value are fitted. The mask is cast to an unsigned char image internally.
- ROI based fitting (`--roibased`): the app fails if no mask is given. The mean curve of all voxels with a non-zero mask value is computed and fitted once. Every output image is filled with the resulting scalar value at all voxels with a mask value greater than zero and with 0 elsewhere. The ROI mean curve is stored in the fit information as well.

The console prints the chosen style ("pixel based" or "ROI based"), the model, the input files and progress percentages during fitting.

### Fitting strategy

All models are fitted with a Levenberg-Marquardt optimizer using the sum of squared differences as criterion. Parameter scales are 1.0 for all parameters. The default constraints of the model factory are applied: for `tofts` and `2CX` the volume fractions are constrained to 0 <= v_e <= 1, 0 <= v_p <= 1 and v_e + v_p <= 1; `descriptive`, `2SL` and `3SL` run unconstrained. The initial parameter values are the defaults of the respective model parameterizer (for example K^trans = 15, v_e = 0.5, v_p = 0.05 for `tofts`; F_p = 20, PS = 5, v_e = 0.1, v_p = 0.04 for `2CX`; A = 1, k_ep = 4, k_el = 0.2, BAT = 1 for `descriptive`). Debug parameter maps of the optimizer are always generated.

The `descriptive` model needs two static parameters: the injection time (`--injectiontime`, in minutes, stored as `Injection_time`) and the pre-contrast signal (`Pre_contrast_signal`). The pre-contrast signal is taken from the first time step of the input image, per voxel in pixel based mode and as the first value of the ROI mean curve in ROI based mode. An injection time of 0 (the default) makes the model evaluation throw an exception, so `-j` must be given for this model.

### Output files

For every result image the app writes `<directory>/<root>_<name>.<extension>`, where `<name>` is the parameter name. Characters that are not allowed in file names (backslash, slash, colon, question mark, double quote, less-than, greater-than, pipe, percent, asterisk and space) are replaced by `_`; other characters such as `^`, `{`, `}` and `-` are kept. The console prints one line per stored image with its type (parameter, derived, criterion, evaluation) and path. Each result image carries the model fit information (model name, fit type, static parameters, AIF or ROI curve, mask UID) as MITK properties; these survive only in formats that store MITK properties such as NRRD.

Independent of the model, the following images are written:

| Type | Name | File suffix |
|------|------|-------------|
| Criterion | `sum_diff^2` | `_sum_diff^2` |
| Evaluation | `Chi^2` (normalized sum of squared differences) | `_Chi^2` |
| Evaluation (debug) | `optimization_time`, `nr_of_iterations`, `stop_condition` | `_optimization_time`, `_nr_of_iterations`, `_stop_condition` |
| Evaluation (debug, `tofts` and `2CX` only) | `constraint_penalty_ratio`, `constraint_failure_ratio`, `constraint_last_failed_parameter` | `_constraint_penalty_ratio`, `_constraint_failure_ratio`, `_constraint_last_failed_parameter` |

The model specific images are:

#### tofts (extended Tofts model)

| Type | Name | Unit | File suffix |
|------|------|------|-------------|
| Parameter | `K^trans` | ml/min/100ml | `_K^trans` |
| Parameter | `v_e` | ml/ml | `_v_e` |
| Parameter | `v_p` | ml/ml | `_v_p` |
| Derived | `k_{e->p}` (= K^trans / v_e) | 1/min | `_k_{e-_p}` |

#### 2CX (two compartment exchange model)

| Type | Name | Unit | File suffix |
|------|------|------|-------------|
| Parameter | `F_p` | ml/min/100ml | `_F_p` |
| Parameter | `PS` | ml/min/100ml | `_PS` |
| Parameter | `v_e` | ml/ml | `_v_e` |
| Parameter | `v_p` | ml/ml | `_v_p` |

#### descriptive (descriptive pharmacokinetic Brix model)

| Type | Name | Unit | File suffix |
|------|------|------|-------------|
| Parameter | `A` | a.u. | `_A` |
| Parameter | `k_{e->p}` | 1/min | `_k_{e-_p}` |
| Parameter | `k_{el}` | 1/min | `_k_{el}` |
| Parameter | `BAT` (bolus arrival time) | min | `_BAT` |

#### 2SL (two step linear model)

The model is `y = y-intercept + slope1 * x` before the change point and `y = y-intercept1 + slope2 * x` after it, with x in seconds.

| Type | Name | File suffix |
|------|------|-------------|
| Parameter | `y-intercept` | `_y-intercept` |
| Parameter | `x_changepoint` | `_x_changepoint` |
| Parameter | `slope1` | `_slope1` |
| Parameter | `slope2` | `_slope2` |
| Derived | `auc` | `_auc` |
| Derived | `y_final` | `_y_final` |
| Derived | `y_max` | `_y_max` |
| Derived | `y-intercept1` | `_y-intercept1` |

#### 3SL (three step linear model)

The model is constant (`baseline`) before the first change point, `y = y-intercept1 + slope1 * x` between the change points and `y = y-intercept2 + slope2 * x` after the second change point, with x in seconds.

| Type | Name | File suffix |
|------|------|-------------|
| Parameter | `baseline` | `_baseline` |
| Parameter | `x_changepoint1` | `_x_changepoint1` |
| Parameter | `x_changepoint2` | `_x_changepoint2` |
| Parameter | `slope1` | `_slope1` |
| Parameter | `slope2` | `_slope2` |
| Derived | `auc` | `_auc` |
| Derived | `x_final` | `_x_final` |
| Derived | `y_final` | `_y_final` |
| Derived | `y_max` | `_y_max` |
| Derived | `y-intercept1` | `_y-intercept1` |
| Derived | `y-intercept2` | `_y-intercept2` |

### Preview mode

With `--preview` the app loads the input image and all masks, computes the AIF and, in ROI based mode, the ROI mean curve, but does not fit. It prints one line per result image with its type and output path, followed by "Preview done.".

### Help text

The app declares a `--help`/`-h` argument, but the short name `-h` is already taken by `--hematocrit`, so the help argument is never registered. `-h` always expects a hematocrit value and `--help` is ignored as an unknown argument. The help text is only printed when the app is started without the required `--input` and `--output` arguments; in that case the app exits with code 1.

### Input requirements

- The input and the AIF image must be 3D+t images. A plain 3D input is rejected because the fitting is only instantiated for four-dimensional images. Any scalar pixel type supported by MITK can be used.
- When an input is DICOM, the reader "MITK DICOM Reader v2 (autoselect)" is preferred.

## Examples

### Extended Tofts fit with an AIF from the same image

```bash
MitkMRPerfusion -i dce_concentration.nrrd -o results/tofts.nrrd -l tofts -n aif_mask.nrrd -m tumor_mask.nrrd
```

Extracts the AIF from `dce_concentration.nrrd` inside `aif_mask.nrrd` (divided by 1 - 0.45), then fits the extended Tofts model to every voxel inside `tumor_mask.nrrd`. Writes `tofts_K^trans.nrrd`, `tofts_v_e.nrrd`, `tofts_v_p.nrrd`, `tofts_k_{e-_p}.nrrd`, `tofts_sum_diff^2.nrrd`, `tofts_Chi^2.nrrd` and the six debug maps into `results`.

### Two compartment exchange model with a separate AIF image

```bash
MitkMRPerfusion -i dce_concentration.nrrd -o results/2cx.nrrd -l 2CX -a aif_concentration.nrrd -n aif_mask.nrrd -h 0.42
```

Computes the AIF from `aif_concentration.nrrd` inside `aif_mask.nrrd` using a hematocrit of 0.42 and fits the two compartment exchange model to all voxels of the input. Produces `2cx_F_p.nrrd`, `2cx_PS.nrrd`, `2cx_v_e.nrrd`, `2cx_v_p.nrrd` and the criterion, evaluation and debug maps.

### ROI based descriptive Brix fit

```bash
MitkMRPerfusion -i dce.nrrd -o results/brix.nrrd -l descriptive -j 0.5 -m tumor_mask.nrrd -r
```

Averages the curves of all voxels in `tumor_mask.nrrd`, fits the Brix model once with an injection time of 0.5 min and writes `brix_A.nrrd`, `brix_k_{e-_p}.nrrd`, `brix_k_{el}.nrrd`, `brix_BAT.nrrd` plus criterion, evaluation and debug maps. Inside the mask every image holds the constant fit result, outside it is 0.

### Preview the outputs of a three step linear fit

```bash
MitkMRPerfusion -i dce.nrrd -o results/3sl.nii.gz -l 3SL -p
```

Loads `dce.nrrd`, prints the output paths of the five parameter images, six derived images, the criterion and the evaluation maps (for example `results/3sl_x_changepoint1.nii.gz`) and exits without fitting.
