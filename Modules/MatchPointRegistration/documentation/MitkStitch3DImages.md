# MitkStitch3DImages User Guide {#MitkStitch3DImagesPage}

[TOC]


## Overview

This command-line application allows users to map and stitch multiple 3D images into a unified output geometry using optional registrations and interpolation strategies.  


## Usage

```bash
MitkStitch3DImages -i <input1> [<input2> ...] -o <output> [options]
```

### Required Arguments

| Argument | Short | Type        | Description |
|----------|-------|-------------|-------------|
| `--inputs` | `-i` | String List | Paths to one or more input 3D image files to be mapped and stitched. |
| `--output` | `-o` | File        | Path to save the fused 3D image output. |

### Optional Arguments

| Argument | Short | Type   | Description |
|----------|-------|--------|-------------|
| `--template` | `-t` | File | Path to a reference image. Its geometry will define the geometry of the stitched output image. |
| `--registrations` | `-r` | String List | Paths to registration files (e.g., from registration frameworks). These are used to map each input image to the reference geometry. Must be same count as `--inputs`. Use an empty string `""` for identity registration. |
| `--interpolator` | `-n` | Int | Interpolation type for image mapping (Default: 2). Values: <br>1: Nearest Neighbour<br>2: Linear<br>3: BSpline 3<br>4: WSinc Hamming<br>5: WSinc Welch |
| `--strategy` | `-s` | Int | Strategy for stitching (Default: 0). Values: <br>0: Mean — computes the mean of overlapping pixel values <br>1: BorderDistance — picks value furthest from input border |
| `--padding` | `-p` | Float | Padding value for areas in the output image not covered by any input (Default: 0.0). |
| `--help` | `-h` | Flag | Display the help text. |

## Notes

- If the `--registrations` parameter is not provided, all images are assumed to use identity mappings.
- If `--registrations` is given, the number of provided registrations **must** match the number of inputs.
- The application assumes inputs and registrations are ordered correspondingly (i.e., N-th input uses N-th registration).

## Example

```bash
MitkStitch3DImages \
  -i scan1.nii scan2.nii \
  -r reg1.map "" \
  -t reference.nii \
  -o stitched_output.nii \
  -n 2 \
  -s 0 \
  -p -1024
```

This will:
- Map `scan1.nii` using `reg1.map`
- Use identity transform for `scan2.nii`
- Stitch both into the geometry defined by `reference.nii`
- Use linear interpolation and mean stitching
- Assign `-1024` to output voxels not covered by any input

## Output

A single 3D image file containing the stitched result of all inputs, saved to the path specified with `--output`.

## Error Handling

- If the number of registrations does not match the number of inputs, the application exits with an error.
- Any exceptions during image loading, registration, or saving are logged and will terminate the application with a failure code.
