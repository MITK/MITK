# MitkStitchImages {#MitkStitch3DImagesPage}

[TOC]

## Overview

MitkStitchImages maps several 3D images into one common output grid and fuses
them into a single image. Each input can be mapped with its own MatchPoint
registration (a `.mapr` file as produced by
[MitkMatchImage](@ref MitkMatchImagePage)) or with the identity transform.
Where inputs overlap, a stitch strategy decides which value the output voxel
receives.

Typical uses are joining scans that cover different parts of the body (e.g.
several stations of a whole-body acquisition) into one volume, or composing
one image out of several registered images. To map a single image use
[MitkMapImage](@ref MitkMapImagePage).

## Usage

```bash
MitkStitchImages -i <input1> [<input2> ...] -o <output> -t <template> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--inputs` | `-i` | String list | Paths of the 3D images that should be mapped and stitched. |
| `--output` | `-o` | File | Path of the stitched result. The extension selects the output format. |
| `--template` | `-t` | File | Image whose geometry defines the output grid. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--registrations` | `-r` | String list | identity | One registration file (`.mapr`) per input, in the same order as `--inputs`. Use `""` for an input that should be mapped with the identity transform. |
| `--interpolator` | `-n` | Int | `2` | Interpolation used for mapping: `1` nearest neighbour, `2` linear, `3` B-spline (order 3), `4` windowed sinc (Hamming), `5` windowed sinc (Welch). |
| `--strategy` | `-s` | Int | `0` | Stitch strategy for overlapping inputs: `0` mean, `1` border distance. |
| `--padding` | `-p` | Float | `0` | Value for output voxels that are not covered by any input. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Inputs and registrations

All inputs must be 3D images without time steps and must share the same pixel
type; otherwise the application fails. If `--registrations` is given, it must
have exactly as many entries as `--inputs`. The n-th registration is applied to
the n-th input. Without `--registrations` every input is mapped with the
identity transform. Every registration must be a 3D-to-3D registration.
Inputs and the template are loaded with the "MITK DICOM Reader v2
(autoselect)" reader preferred for DICOM data.

The template image's geometry (origin, spacing, size, orientation) defines the
output grid. The template pixel values are not used.

### Stitch strategies

- `0` (mean): the output voxel receives the mean of all input values that map
  to it.
- `1` (border distance): the output voxel receives the value of the input
  whose sampled position has the largest minimal distance (in mm) to its own
  image border. This favours the input in which the position is most central
  and suppresses border artifacts in the overlap region.

Any value other than `0` is treated like `1`.

### Interpolation and padding

The selected interpolator is used for mapping every input. Any interpolator
code other than 1 to 5 falls back to linear interpolation. Output voxels that
are covered by no input receive the padding value.

## Examples

### Stitch two aligned stations without registration

```bash
MitkStitchImages -i station1.nrrd station2.nrrd -t wholebody_template.nrrd -o wholebody.nrrd
```

Both inputs are already aligned in world coordinates, so the identity
transform is used. Overlapping voxels receive the mean of both inputs.

### Stitch with registrations and border distance strategy

```bash
MitkStitchImages -i scan_a.nrrd scan_b.nrrd scan_c.nrrd -r a_to_ref.mapr "" c_to_ref.mapr -t reference.nrrd -o stitched.nrrd -s 1
```

`scan_a.nrrd` and `scan_c.nrrd` are mapped with their registrations,
`scan_b.nrrd` with the identity transform. In overlapping regions the value of
the input that is farthest from its own border wins.

### CT stitching with air padding and B-spline interpolation

```bash
MitkStitchImages -i ct_upper.nrrd ct_lower.nrrd -r upper_to_ref.mapr lower_to_ref.mapr -t ct_reference.nrrd -o ct_stitched.nrrd -n 3 -p -1024
```

Uses third order B-spline interpolation for mapping and fills voxels covered
by neither input with `-1024`.
