# MitkCurveDescriptor {#MitkCurveDescriptorPage}

[TOC]

## Overview

`MitkCurveDescriptor` computes model-free curve descriptor maps for a dynamic (3D+t) image. For every voxel the time curve is evaluated and the area under the curve (AUC), the area under the first moment curve (AUMC), the mean residence time (MRT), the time to peak and the peak value are written as separate 3D parameter images. The time axis is taken from the time geometry of the input image and is used in seconds.

The app expects a 3D+t image as input (for example a DCE-MRI series) and optionally a 3D mask that restricts the computation to a region of interest. Use it when a quick, model-free characterization of contrast agent kinetics is sufficient. For model-based perfusion analysis see [MitkMRPerfusion](@ref MitkMRPerfusionPage); to convert MR signal into concentration beforehand see [MitkMRSignal2Concentration](@ref MitkMRSignal2ConcentrationPage). If the dynamic series is only available as separate 3D volumes, [MitkFuse3Dto4DImage](@ref MitkFuse3Dto4DImagePage) can assemble them into a 3D+t image first.

This app is built with the CMake option `BUILD_PerfusionCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkCurveDescriptor -i <input> -o <output> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Input 3D+t image. |
| `--output` | `-o` | File | Output file template. Directory and extension determine where and in which format the parameter images are written; the file name without extension is used as root name, and a suffix is added for each descriptor. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--mask` | `-m` | File | | 3D mask that defines the voxels for which descriptors are computed. Must have the same geometry as the input image. |
| `--preview` | `-p` | Flag | | Only print the output file paths the app would produce with the current settings; nothing is loaded or computed. |
| `--verbose` | `-v` | Flag | | Accepted, but currently has no effect on the output. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Computed descriptors and output files

All descriptors are computed voxel-wise from the time curve of the input image. The time grid is extracted from the time geometry of the input image and converted to seconds, so all time-related descriptors are in seconds. The output images are floating point images with the geometry of one time step of the input.

| Descriptor | Suffix | Description |
|------------|--------|-------------|
| Area under the curve | `_AUC` | Trapezoidal integral of the absolute curve values over time. |
| Area under the first moment curve | `_AUMC` | Trapezoidal integral of the absolute value of curve value times time. |
| Mean residence time | `_MRT` | Ratio AUMC / AUC. |
| Time to peak | `_TimeToPeak_Time` | Time point (in seconds) of the first maximum of the curve. |
| Peak value | `_TimeToPeak_Peak` | Value of the curve at that maximum. |

The output path of each image is `<directory>/<root>_<suffix>.<extension>` derived from `--output`. For `-o results/dce.nrrd` the app writes `results/dce_AUC.nrrd`, `results/dce_AUMC.nrrd`, `results/dce_MRT.nrrd`, `results/dce_TimeToPeak_Time.nrrd` and `results/dce_TimeToPeak_Peak.nrrd`. The set of descriptors is fixed; there is no argument to select a subset.

### Input requirements

- The input must be a 3D+t image. A plain 3D image is rejected with an exception because the computation is only instantiated for four-dimensional images. Any scalar pixel type supported by MITK can be used.
- When the input is DICOM, the reader "MITK DICOM Reader v2 (autoselect)" is preferred.
- Progress is printed to the console as a sequence of percentages.

### Mask handling

- Only voxels with a non-zero mask value are processed. The mask is cast to an unsigned char image internally.
- The mask is used as passed. It must be a 3D image; a 3D+t mask is rejected because the mask preparation is only instantiated for three-dimensional images. This differs from [MitkMRPerfusion](@ref MitkMRPerfusionPage), which reduces a 3D+t mask to its first time step.
- Without a mask all voxels of the image are processed.

### Preview mode

With `--preview` the app does not load the input or mask and does not compute anything. It only prints one line per descriptor with the resulting output path, which is useful to check the naming before starting a long computation.

## Examples

### Compute descriptor maps for a whole image

```bash
MitkCurveDescriptor -i dce.nrrd -o results/dce.nrrd
```

Processes every voxel of `dce.nrrd` and writes `dce_AUC.nrrd`, `dce_AUMC.nrrd`, `dce_MRT.nrrd`, `dce_TimeToPeak_Time.nrrd` and `dce_TimeToPeak_Peak.nrrd` into the `results` directory.

### Restrict the computation to a tumor mask

```bash
MitkCurveDescriptor -i dce.nrrd -o results/tumor.nrrd -m tumor_mask.nrrd
```

Only voxels with a non-zero value in `tumor_mask.nrrd` are evaluated. The five descriptor images are written with the root name `tumor` in NRRD format.

### Preview the output file names

```bash
MitkCurveDescriptor -i dce.nrrd -o results/dce.nii.gz -p
```

Prints the five output paths (for example `results/dce_AUC.nii.gz`) without loading or processing any data.
