# MitkFuse3Dto4DImage {#MitkFuse3Dto4DImagePage}

[TOC]

## Overview

MitkFuse3Dto4DImage fuses several 3D images with the same geometry into one 3D+t (4D) image that MITK treats as dynamic data. Each input image becomes one time step of the output, in the order in which the inputs are given. Optionally, the time bounds of the time steps can be specified in milliseconds.

Use it to build a dynamic dataset from individually stored time frames, for example after processing the frames one by one. The reverse operation is provided by [MitkSplit4Dto3DImages](@ref MitkSplit4Dto3DImagePage). If the input images do not share the same geometry, resample them first, e.g. with [MitkMapImage](@ref MitkMapImagePage) or [MitkMatchImage](@ref MitkMatchImagePage).

## Usage

```bash
MitkFuse3Dto4DImage -i <input1> <input2> ... -o <output> [-t <t0> <t1> ... <tN>]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--inputs` | `-i` | String list | Paths to the 3D images that should be fused, in time step order. All images must have the same geometry and pixel type. |
| `--output` | `-o` | File | Path to the fused 3D+t image. The extension selects the output format. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--time` | `-t` | String list | `0 1 2 ... N` | Time bounds of the resulting time geometry in milliseconds. The first number is the start of the first time step, every further number is the upper bound of one time step, so N inputs need N+1 numbers (see Details). |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Time geometry

The output gets an arbitrary time geometry with one time step per input image:

- Without `--time`, the time steps are `[0, 1)`, `[1, 2)`, ..., `[N-1, N)` ms, i.e. every time step has a duration of 1 ms.
- With `--time`, exactly N+1 numbers are required for N input images: the first is the minimum bound of the first time step, all following numbers are the maximum bounds of the successive time steps. `-t 2 3.5 10` with two inputs yields the time steps `[2, 3.5)` and `[3.5, 10)` ms. Any other count of numbers is an error and the app exits with 1.

Numbers are parsed with the C locale, so use a period as decimal separator. A value that cannot be parsed as a number is rejected with an error and the app exits with 1.

### Geometry, pixel type, and properties

All inputs must have the same geometry (origin, spacing, orientation, and size within MITK's default tolerance) and the same pixel type as the first input; otherwise the app aborts with an error. The inputs are expected to be 3D images.

The output takes its properties (metadata) from the first input. Properties of the other inputs are only added if the first input does not have a property of the same name, with one exception: time-resolved string properties (as written by the DICOM reader) are merged so that the value of input k ends up at time step k of the output.

### Input loading

Inputs are loaded with the MITK DICOM reader preferred (`MITK DICOM Reader v2 (autoselect)`), so a DICOM file can be given as input directly; for other formats the default reader is used.

## Examples

### Fuse three frames with the default time geometry

```bash
MitkFuse3Dto4DImage -i frame0.nrrd frame1.nrrd frame2.nrrd -o dynamic.nrrd
```

Creates `dynamic.nrrd` with three time steps covering `[0, 1)`, `[1, 2)`, and `[2, 3)` ms.

### Fuse two frames with explicit acquisition times

```bash
MitkFuse3Dto4DImage -i pre_contrast.nii.gz post_contrast.nii.gz -o perfusion.nrrd -t 0 30.5 65
```

Creates `perfusion.nrrd` with two time steps, `[0, 30.5)` and `[30.5, 65)` ms.

### Wrong number of time bounds

```bash
MitkFuse3Dto4DImage -i frame0.nrrd frame1.nrrd frame2.nrrd -o dynamic.nrrd -t 0 1
```

Three inputs need four time bounds; the app prints an error and exits with 1 without loading the images.
