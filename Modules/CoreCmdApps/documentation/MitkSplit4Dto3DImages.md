# MitkSplit4Dto3DImages {#MitkSplit4Dto3DImagePage}

[TOC]

## Overview

MitkSplit4Dto3DImages splits a 3D+t (4D) image into one 3D image per time step. Each time step is written to its own file, and the image properties (metadata) are transferred to the respective outputs; time-resolved properties are reduced to the value of the corresponding time step.

Use it when time frames of a dynamic image have to be processed individually, for example with tools that only accept 3D images such as [MitkImageTypeConverter](@ref MitkImageTypeConverterPage). The reverse operation is provided by [MitkFuse3Dto4DImage](@ref MitkFuse3Dto4DImagePage).

## Usage

```bash
MitkSplit4Dto3DImages -i <input> -o <output pattern>
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Path to the image that should be split. If the image has only one time step it is stored as output untouched. |
| `--output` | `-o` | File | Path of the output images. If the input has several time steps the path is used as a pattern and the suffix `_<time step>` is inserted before the extension. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Output naming

If the input has more than one time step, the output files are named `<name>_<t><extension>` with `t` counting from 0. For `-o frame.nrrd` and three time steps this yields `frame_0.nrrd`, `frame_1.nrrd`, and `frame_2.nrrd`. The output format is determined by the extension of the pattern.

If the input has exactly one time step, nothing is split: the loaded image is written as is to the output path, without suffix.

### Property handling

Every output image receives the properties of the input image:

- Properties that are not time-resolved are cloned unchanged.
- Time-resolved string properties (`TemporoSpatialStringProperty`, as written by the DICOM reader) are reduced to the values of the respective time step, which becomes time step 0 in the output. If such a property has no value for a time step, it is omitted from that output.

### Input loading

The input is loaded with the MITK DICOM reader preferred (`MITK DICOM Reader v2 (autoselect)`), so a dynamic DICOM series can be given as input directly; for other formats the default reader is used. If loading or saving fails, an error is printed and the app exits with 1.

## Examples

### Split a dynamic image into frames

```bash
MitkSplit4Dto3DImages -i perfusion.nrrd -o frame.nrrd
```

For an input with 20 time steps this writes `frame_0.nrrd` to `frame_19.nrrd`, each a 3D image carrying the metadata of its time step.

### Split a dynamic DICOM series into NIfTI files

```bash
MitkSplit4Dto3DImages -i ./dce_mri/IM0001.dcm -o ./frames/dce.nii.gz
```

The DICOM series containing `IM0001.dcm` is loaded as one dynamic image and every time step is written to `./frames/dce_<t>.nii.gz`.

### Input with a single time step

```bash
MitkSplit4Dto3DImages -i static.nrrd -o result.nrrd
```

The image has only one time step, so it is written unchanged to `result.nrrd`.
