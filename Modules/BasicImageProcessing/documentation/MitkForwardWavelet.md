# MitkForwardWavelet {#MitkForwardWaveletPage}

[TOC]

## Overview

MitkForwardWavelet computes a forward wavelet decomposition of an image with an isotropic wavelet in the frequency domain. For every level a set of high-pass sub-band images is produced, plus one low-pass image at the coarsest level. Wavelet family and border condition for the FFT padding can be chosen.

This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkForwardWavelet -i <image> -o <output prefix> -e <extension> -levels <n> -bands <m> -w <wavelet> -border <condition>
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | File | Input image. |
| `--output` | `-o` | File | Prefix of the output files, including the path. |
| `--output-extension` | `-e` | File | File extension of the output files, including the dot, e.g. `.nrrd`. |
| `--number-of-levels` | `-levels` | Int | Number of decomposition levels. |
| `--number-of-bands` | `-bands` | Int | Number of high-pass sub-bands per level. |
| `--wavelet` | `-w` | Int | Wavelet family: 0 Shannon, 1 Simoncelli, 2 Vow, 3 Held. Other values select Shannon. |
| `--border-condition` | `-border` | Int | Padding at the image border: 0 constant (zero), 1 periodic, 2 zero-flux Neumann. Other values select constant. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Output file names

Files are named `<output prefix><index><extension>` with a running index starting at 0, in the order the sub-bands are produced: the sub-bands of each level followed by the final low-pass image. With n levels and m bands, n * m + 1 files are written. Before the transform the image is padded to a power of two voxels per dimension (using the chosen border condition), so the sub-bands of the first level are at least as large as the input. Every further level halves the number of voxels and doubles the voxel spacing. The output images are double precision images.

The first object in the input file is used. If it is not an image, the app prints a message and exits with code 0 without writing an output.

## Examples

### Two levels, one band, Held wavelet

```bash
MitkForwardWavelet -i ct.nrrd -o wavelet/ct_ -e .nrrd -levels 2 -bands 1 -w 3 -border 0
```

Writes `wavelet/ct_0.nrrd`, `wavelet/ct_1.nrrd` (high-pass of level 1 and 2) and `wavelet/ct_2.nrrd` (low-pass).

### Simoncelli wavelet with periodic padding

```bash
MitkForwardWavelet -i mr.nrrd -o mr_sim -e .nii.gz -levels 3 -bands 2 -w 1 -border 1
```

Writes seven files, `mr_sim0.nii.gz` to `mr_sim6.nii.gz`.
