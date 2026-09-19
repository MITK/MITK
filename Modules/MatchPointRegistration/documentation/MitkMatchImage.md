# MitkMatchImage {#MitkMatchImagePage}

[TOC]

## Overview

MitkMatchImage registers a moving image onto a target image with a deployed
MatchPoint registration algorithm and stores the resulting registration in a
file. It does not resample the moving image; the registration file is meant to
be applied afterwards with [MitkMapImage](@ref MitkMapImagePage) (map a single
image or segmentation) or [MitkStitchImages](@ref MitkStitch3DImagesPage) (map
and fuse several images into one).

The registration algorithm is not built into the application. It is loaded at
runtime from a MatchPoint deployment library (a `.dll`, `.so` or `.dylib`
file). MITK ships a set of such libraries next to its executables; their file
names start with `mdra-` (e.g. `mdra-<version>_MITK_MultiModal_rigid_default`).
Optional masks restrict the image regions the algorithm evaluates, and
algorithm parameters can be set through a JSON string.

## Usage

```bash
MitkMatchImage -m <moving> -t <target> -a <algorithm library> -o <registration.mapr> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--moving` | `-m` | File | Image that should be registered onto the target. |
| `--target` | `-t` | File | Image that defines the target space of the registration. |
| `--algorithm` | `-a` | File | Deployed MatchPoint registration algorithm library. |
| `--output` | `-o` | File | Registration file to write. Use the `.mapr` extension. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--moving_mask` | `-mm` | File | | Mask (image or multi-label segmentation) that restricts the moving image region used for registration. |
| `--moving_mask_label` | `-mml` | String | | Name of the label to use if the moving mask is a multi-label segmentation with more than one label. |
| `--target_mask` | `-tm` | File | | Mask (image or multi-label segmentation) that restricts the target image region used for registration. |
| `--target_mask_label` | `-tml` | String | | Name of the label to use if the target mask is a multi-label segmentation with more than one label. |
| `--parameters` | `-p` | String | | JSON object with algorithm parameters as key-value pairs. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Algorithm library

The library must be a MatchPoint deployment library (built with MatchPoint's
deployment support). On start the application prints the algorithm identifier
(UID) of the loaded library and reports algorithm events (initializing,
starting, stopping condition, finalizing) on the console while the
registration runs.

Moving and target images are passed to the algorithm as they are if the
algorithm accepts their pixel types. Otherwise they are cast to the pixel type
the algorithm expects. Algorithms that cannot work on images at all cause the
application to fail.

### Time steps

Only the first time step of the moving and of the target image is used. If an
image has more than one time step, this is reported on the console.

### Masks

A mask file may contain a plain image or an MITK multi-label segmentation.
For a multi-label segmentation the mask is derived as follows:

- exactly one label: that label is used;
- several labels and no label name given: the first label is used and a
  message is printed;
- several labels and a label name given: the label with that name is searched
  in all groups. If no label or more than one label has that name, the
  application fails.

Masks are only passed on if the algorithm implements MatchPoint's masked
registration interface and the mask dimension matches the algorithm's
dimension. Otherwise the masks are ignored without a message.

### Parameters

The value of `--parameters` must be a JSON object. Each key must name a
writable meta property of the loaded algorithm. The application fails with a
message if the JSON string cannot be parsed, if a key is unknown to the
algorithm (the message lists all known property names), if the property is not
writable, or if the value cannot be converted to the property type. Supported
property types are integers (`int`, `unsigned int`, `long`, `unsigned long`),
floating point values (`float`, `double`), `bool`, strings, and arrays of
`double`. If the algorithm does not support meta properties at all, the
parameters are ignored with a warning.

Which properties an algorithm offers depends on the library. Passing an
unknown key is a convenient way to get the list of known properties printed.

### Output

The registration is written with the MatchPoint registration writer, which is
selected by the `.mapr` extension. The file contains the registration only,
not a resampled image.

## Examples

### Rigid registration with a shipped algorithm

```bash
MitkMatchImage -m pet.nrrd -t ct.nrrd -a mdra-0-14_MITK_MultiModal_rigid_default.dll -o pet_to_ct.mapr
```

Registers `pet.nrrd` onto `ct.nrrd` with the shipped multi-modal rigid
algorithm (adjust the version part of the file name to your installation) and
writes the registration to `pet_to_ct.mapr`.

### Masked registration with multi-label segmentations

```bash
MitkMatchImage -m followup_ct.nrrd -t baseline_ct.nrrd -a mdra-0-14_MITK_MultiModal_rigid_default.dll -o followup_to_baseline.mapr -mm followup_seg.nrrd -mml "Liver" -tm baseline_seg.nrrd -tml "Liver"
```

Both masks are multi-label segmentations; the label named `Liver` is
extracted from each and passed to the algorithm as moving and target mask.

### Passing algorithm parameters

```bash
MitkMatchImage -m t2.nrrd -t t1.nrrd -a mdra-0-14_MITK_MultiModal_affine_default.dll -o t2_to_t1.mapr -p "{\"MaximumIterations\": 500}"
```

Sets the meta property `MaximumIterations` of the algorithm before the
registration starts. The property names depend on the algorithm; the
application prints the known names if an unknown key is passed.
