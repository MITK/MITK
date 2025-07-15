# MitkMapImage User Guide {#MitkMapImagePage}

[TOC]

## Overview
This tool maps a medical image or segmentation into a target geometry using an optional registration file and image interpolation method. It supports optional output geometry templates, padding, and super-sampling.

## Usage

```bash
MitkMapImage -i <inputImage> -o <outputImage> [options]
```

### Required Arguments

| Argument           | Short | Type   | Description                               |
|--------------------|-------|--------|-------------------------------------------|
| `--input`          | `-i`  | File   | Input image file to be mapped             |
| `--output`         | `-o`  | File   | Output path for the mapped image          |

---

### Optional Arguments

| Argument           | Short | Type     | Description                                                                 |
|--------------------|-------|----------|-----------------------------------------------------------------------------|
| `--registration`   | `-r`  | File     | Path to registration file (default: identity)                              |
| `--template`       | `-t`  | File     | Reference image that defines output geometry (default: input geometry)     |
| `--interpolator`   | `-n`  | Integer  | Interpolation type (default: 2)                                            |
|                    |       |          | 1: Nearest Neighbour                                                        |
|                    |       |          | 2: Linear                                                                    |
|                    |       |          | 3: BSpline3                                                                  |
|                    |       |          | 4: WSinc Hamming                                                             |
|                    |       |          | 5: WSinc Welch                                                               |
| `--padding`        | `-p`  | Float    | Padding value for unmapped voxels (default: 0.0)                            |
| `--super-sampling` | `-s`  | List     | Super-sampling factor(s): 1 or 3 values allowed                             |
| `--help`           | `-h`  | Flag     | Print help information and exit                                             |

## Examples

### Map an image using a registration and reference image

```bash
MitkMapImage -i fixedImage.nii -r deformableReg.mapr -t refImage.nii -o outputImage.nii
```

---

### Map using identity (no registration) and linear interpolation (default)

```bash
MitkMapImage -i input.nii -o output.nii
```

---

### Use BSpline interpolation and pad unmapped voxels with -1000

```bash
MitkMapImage -i input.nii -o output.nii  -r reg.mapr -n 3 -p -1000
```

---

### Apply super-sampling (doubles resolution isotropically)

```bash
MitkMapImage -i input.nii -o output.nii -s 2
```

---

### Anisotropic super-sampling

```bash
MitkMapImage -i input.nii -o output.nii -s 2 2 1
```

---

## 🔍 Notes

- Tool does also support mapping of dynamic data (e.g. 3D+t images).
- Supports both standard images and multi-label segmentations.
- If the registration file is missing, an identity transform is used.
- Super-sampling must either provide **one** value (isotropic) or **three** (for x, y, z).
- If mapping a segmentation, the interpolaiton option will be ignored. Nearest neighbour will always be used.
