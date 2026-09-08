# MitkMultiResolutionPyramid {#MitkMultiResolutionPyramidPage}

[TOC]

## Overview

MitkMultiResolutionPyramid computes a Gaussian multi-resolution pyramid of an image. Each level halves the resolution of the previous one. One image per level is written.

This app is built with the CMake option `BUILD_BasicImageProcessingCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkMultiResolutionPyramid -i <image> -o <output prefix> -e <extension> -levels <n> [--as-double]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | File | Input image. |
| `--output` | `-o` | File | Prefix of the output files, including the path. |
| `--output-extension` | `-e` | File | File extension of the output files, including the dot, e.g. `.nrrd`. |
| `--number-of-levels` | `-levels` | Int | Number of pyramid levels. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--as-double` | `-double` | Flag | off | Write the results with pixel type double instead of the input pixel type. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Output file names

Files are named `<output prefix><level><extension>`. Level 0 is the highest resolution and level n-1 the lowest, where n is the number of levels. For `-o pyramid/ct -e .nrrd -levels 3` the files `pyramid/ct0.nrrd`, `pyramid/ct1.nrrd` and `pyramid/ct2.nrrd` are written, with `ct2.nrrd` being the coarsest.

The first object in the input file is used. If the file cannot be read or does not contain an image, the app reports the error and exits with code 1.

## Examples

### Three-level pyramid

```bash
MitkMultiResolutionPyramid -i ct.nrrd -o ct_level -e .nrrd -levels 3
```

Writes `ct_level0.nrrd` (full resolution), `ct_level1.nrrd` (half) and `ct_level2.nrrd` (quarter).

### Pyramid with double output in NIfTI format

```bash
MitkMultiResolutionPyramid -i mr.nrrd -o mr_pyr -e .nii.gz -levels 4 --as-double
```
