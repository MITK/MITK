# MitkMapImage {#MitkMapImagePage}

[TOC]

## Overview

MitkMapImage resamples an image or a multi-label segmentation into the
geometry of a template image, optionally applying a MatchPoint registration
(a `.mapr` file as produced by [MitkMatchImage](@ref MitkMatchImagePage)).
Without a registration the identity transform is used, which turns the
application into a plain resampling tool.

Use it to bring a moving image into the space of a target image after a
registration, to resample data onto the grid of another image, or to increase
the resolution of the output grid with super-sampling. To fuse several images
into one output at once, use [MitkStitchImages](@ref MitkStitch3DImagesPage)
instead.

## Usage

```bash
MitkMapImage -i <input> -o <output> -t <template> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Image or multi-label segmentation that should be mapped. |
| `--output` | `-o` | File | Path of the mapped result. The extension selects the output format. |
| `--template` | `-t` | File | Image whose geometry defines the output grid. See Details for why this is required. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--registration` | `-r` | File | identity | Registration file (`.mapr`) used for mapping. If omitted, an identity transform is used. |
| `--interpolator` | `-n` | Int | `2` | Interpolation for images: `1` nearest neighbour, `2` linear, `3` B-spline (order 3), `4` windowed sinc (Hamming), `5` windowed sinc (Welch). Ignored for segmentations. |
| `--padding` | `-p` | Float | `0` | Value for output voxels that are not covered by the input image. Ignored for segmentations. |
| `--super-sampling` | `-s` | String list | | One factor (isotropic) or three factors (x, y, z) that refine the template grid. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Template

The help text of the application states that the input geometry is used when
no template is given. The argument is nevertheless declared as required, so
the parser rejects any call without `--template`. Pass the input image itself
as template if you want to keep its geometry.

Only the spatial geometry of the template's first time step is used. The time
geometry (number and duration of time steps) is taken from the input.

### Input types and 3D+t data

The input file may contain an image or an MITK multi-label segmentation. If
loading the file yields more than one object, only the first one is mapped
(a warning is printed). DICOM template images are loaded with the
"MITK DICOM Reader v2 (autoselect)" reader.

Dynamic (3D+t) inputs are supported: every time step is mapped with the same
registration into the same output grid.

### Registration

The registration must map from the input (moving) space to the template
(target) space, which is what MitkMatchImage produces when the template is the
target image of the registration. Output voxels that the registration cannot
map at all cause the application to fail; output voxels that map outside the
input image receive the padding value.

### Images versus segmentations

Images are resampled with the selected interpolator and padded with
`--padding`. Any interpolator code other than 1 to 5 falls back to linear
interpolation. Images with composite pixel types (RGB, RGBA) only support
nearest neighbour and linear interpolation; other codes fall back to linear.

Multi-label segmentations are always mapped group by group with nearest
neighbour interpolation, uncovered voxels get the label value 0 (background),
and the label definitions of the input are carried over. `--interpolator` and
`--padding` have no effect on segmentations.

### Super-sampling

`--super-sampling` takes one or three non-negative integers. One value is used
for all three axes. The template spacing is divided by the factor and the
number of voxels multiplied accordingly, so `2` doubles the resolution along
every axis. A single factor of `1` disables super-sampling. Any other number of
values is rejected.

## Examples

### Map an image with a registration into the target space

```bash
MitkMapImage -i pet.nrrd -r pet_to_ct.mapr -t ct.nrrd -o pet_in_ct_space.nrrd
```

Applies the registration `pet_to_ct.mapr` to `pet.nrrd` and resamples the
result onto the grid of `ct.nrrd` with linear interpolation.

### Resample onto another grid without a registration

```bash
MitkMapImage -i t2.nrrd -t t1.nrrd -o t2_resampled.nrrd
```

Uses the identity transform and resamples `t2.nrrd` onto the grid of
`t1.nrrd`. Both images are expected to be already aligned in world
coordinates.

### Map a CT with B-spline interpolation and air padding

```bash
MitkMapImage -i followup_ct.nrrd -r followup_to_baseline.mapr -t baseline_ct.nrrd -o followup_ct_mapped.nrrd -n 3 -p -1024
```

Uses third order B-spline interpolation and fills voxels outside the moving
image with `-1024`.

### Map a segmentation and double the resolution

```bash
MitkMapImage -i liver_seg.nrrd -r followup_to_baseline.mapr -t baseline_ct.nrrd -o liver_seg_mapped.nrrd -s 2 2 1
```

The multi-label segmentation is mapped with nearest neighbour interpolation
into a grid with half the spacing of `baseline_ct.nrrd` in x and y and the
original spacing in z.
