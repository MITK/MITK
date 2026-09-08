# MitkMRSignal2Concentration {#MitkMRSignal2ConcentrationPage}

[TOC]

## Overview

`MitkMRSignal2Concentration` converts a dynamic (3D+t) MR signal image into a contrast agent concentration image. Three conversion modes are available: T1 absolute signal enhancement, T1 relative signal enhancement and T2 (T2*) signal conversion. In all modes the first time step of the input serves as pre-contrast baseline and the conversion is applied voxel-wise to every time step.

The resulting concentration image is the expected input of [MitkMRPerfusion](@ref MitkMRPerfusionPage), which assumes that both the tissue image and the AIF image already contain concentration values. It can also be used before [MitkCurveDescriptor](@ref MitkCurveDescriptorPage) if descriptors of the concentration curve rather than of the raw signal are wanted.

This app is built with the CMake option `BUILD_PerfusionCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkMRSignal2Concentration -i <input> -o <output> (--t1-absolute | --t1-relative | --t2) -k <factor> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Input 3D+t MR signal image. |
| `--output` | `-o` | File | Output file for the concentration image. The extension determines the format. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--t1-absolute` | | Flag | | Convert using T1 absolute signal enhancement. Exactly one of the three mode flags must be given. |
| `--t1-relative` | | Flag | | Convert using T1 relative signal enhancement. Exactly one of the three mode flags must be given. |
| `--t2` | | Flag | | Convert a T2 (T2*) weighted signal to concentration. Exactly one of the three mode flags must be given. |
| `--k` | `-k` | Float | | Conversion factor k. Required in all three modes; the app fails if it is missing or 0. |
| `--te` | | Float | | Echo time TE. Required in `--t2` mode; the app fails if it is missing or 0 in that mode. Ignored otherwise. |
| `--recovery-time` | | Float | | Accepted, but not used by any available conversion mode. |
| `--relaxivity` | | Float | | Accepted, but not used by any available conversion mode. |
| `--relaxation-time` | | Float | | Accepted, but not used by any available conversion mode. |
| `--verbose` | `-v` | Flag | | Accepted, but currently has no effect on the output. |
| `--help` | `-h` | Flag | | Show the help text and exit. Only effective if the other arguments form a valid call, see below. |

## Details

### Conversion modes

Exactly one mode flag must be set. With no mode flag the app fails with "Please select the type of conversion", with more than one it fails with "Please select only ONE type of conversion". For each voxel and time step the signal `S(t)` is converted using the baseline signal `S0` of the same voxel:

| Mode | Formula | Notes |
|------|---------|-------|
| `--t1-absolute` | `C(t) = k * (S(t) - S0)` | |
| `--t1-relative` | `C(t) = k * (S(t) - S0) / S0` | Result is 0 where `S0` is 0. |
| `--t2` | `C(t) = -(k / TE) * ln(S(t) / S0)` | Result is 0 where `S(t)` or `S0` is 0. |

The help text of the app mentions a "T1-flash" mode for `--recovery-time`, `--relaxivity` and `--relaxation-time`. This mode is not available in the app; the three values are parsed but never passed on, so they have no effect.

### Argument validation and defaults

- `k` and `te` are validated before anything is loaded. The help text reports a default of 1 for both, but the app uses 0 when the argument is absent and then rejects the call ("Please set 'k'" or "Please set 'te'"). Effectively `-k` is always required, and `--te` is required in `--t2` mode.
- Validation happens before the `--help` check. `MitkMRSignal2Concentration --help` alone therefore prints the help text because the required arguments are missing, but exits with code 1. A call with `-i`, `-o`, a mode flag, `-k` (and `--te` for `--t2`) plus `--help` prints the help text and exits with code 0.

### Baseline and output

- The baseline `S0` is the first time step of the input image. The app does not expose a way to average several pre-contrast time steps.
- The input must be a 3D+t image; a plain 3D image is rejected because the conversion is only instantiated for four-dimensional images. Any scalar pixel type supported by MITK can be used.
- The output is a 3D+t image with double precision pixels and the time geometry of the input. The property list of the input (for example DICOM tags) is copied to the output so that downstream tools can access it.
- When the input is DICOM, the reader "MITK DICOM Reader v2 (autoselect)" is preferred.
- The output path is used as given; no suffix is appended. The console prints "Store result: <output>" when the image has been written.

## Examples

### T1 absolute signal enhancement

```bash
MitkMRSignal2Concentration -i dce.nrrd -o dce_concentration.nrrd --t1-absolute -k 1
```

Subtracts the first time step from every time step of `dce.nrrd` and scales the difference by 1. Writes the 4D concentration image `dce_concentration.nrrd`.

### T1 relative signal enhancement with a calibration factor

```bash
MitkMRSignal2Concentration -i dce.nrrd -o dce_concentration.nrrd --t1-relative -k 0.25
```

Computes the relative enhancement `(S(t) - S0) / S0` per voxel, multiplies it by 0.25 and stores the result. Voxels with a zero baseline get a concentration of 0.

### T2* weighted conversion

```bash
MitkMRSignal2Concentration -i dsc.nii.gz -o dsc_concentration.nii.gz --t2 -k 1 --te 30
```

Converts the dynamic susceptibility contrast series `dsc.nii.gz` using `C(t) = -(1 / 30) * ln(S(t) / S0)` and writes the result as compressed NIfTI.
