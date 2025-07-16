# MitkFuse3Dto4DImage User Guide {#MitkFuse3Dto4DImagePage}

[TOC]

## Overview

`MitkFuse3Dto4DImage` is a command-line tool provided by the Medical Imaging Interaction Toolkit (MITK) for fusing a series of 3D images into a 4D image (3D + time). This is particularly useful when creating dynamic datasets from individual time-resolved image acquisitions.

The resulting 4D image can then be further processed as dynamic data in MITK or other compatible platforms.


## Usage

```bash
MitkFuse3Dto4DImage -i <input1> <input2> ... -o <output> [--time <t0> <t1> <t2> ...]
```

---

## Required Arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--inputs` | `-i` | List of files | Paths to input 3D image files to be fused. Must have the same geometry. |
| `--output` | `-o` | File | Output file path for the generated 4D image. |


## Optional Arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--time` | `-t` | List of numbers | Time bounds in milliseconds. Format: `[minBound0 maxBound1 [maxBound2 [...]]]`. See examples below. If omitted, a default time geometry is generated with 1 ms per step. |
| `--help` | `-h` | Flag | Show help and usage information. |


## Details

### Time Geometry Explanation

- You must provide **N+1** time bounds for **N** input images.
- The first number defines the **start time** of the first time step.
- Every subsequent number is a **max bound** for the preceding time step.

For example, `--time 2 3.5 10` used with 2 input images defines:
- Time step 0: from 2 ms to 3.5 ms
- Time step 1: from 3.5 ms to 10 ms

If the number of time bounds does not match the requirement (`number_of_inputs + 1`), the tool will produce an error.

## Examples

### Example 1: Basic usage with default time geometry

```bash
MitkFuse3Dto4DImage -i image1.nii image2.nii image3.nii -o output4D.nrrd
```

**What happens:**  
- Fuses `image1.nii`, `image2.nii`, and `image3.nii` into a 4D image.  
- A default time geometry with start = 0 ms and 1 ms duration per frame is created: `[0 1 2 3]`.

**Result:**  
- 4D output: `output4D.nrrd`  
- Time steps:  
  - Step 0: [0 ms – 1 ms]  
  - Step 1: [1 ms – 2 ms]  
  - Step 2: [2 ms – 3 ms]

---

### Example 2: Usage with explicit time geometry

```bash
MitkFuse3Dto4DImage -i image1.nii image2.nii -o output4D.nrrd -t 5 10 20
```

**What happens:**  
- Fuses two input images.  
- Time geometry explicitly defined:  
  - Step 0: [5 ms – 10 ms]  
  - Step 1: [10 ms – 20 ms]

**Result:**  
- 4D output: `output4D.nrrd`  
- Contains two time steps with user-defined timing.

---

### Example 3: Error due to incorrect time bounds

```bash
MitkFuse3Dto4DImage -i image1.nii image2.nii image3.nii -o output4D.nrrd -t 0 1
```

**What happens:**  
- Input: 3 images → needs 4 time bounds.  
- Only 2 time bounds provided → the application exits with an error.

---

## Supported Input Formats

All image formats supported by MITK, e.g.:
- NIfTI (.nii, .nii.gz)
- NRRD (.nrrd)
- DICOM (folder or series)

**CAUTION**: All input images must have **identical geometry** (size, spacing, orientation).
If the geometries are not equal One can use the apps MITKMapImage and MITKMatchImage to map all images into one referenc geometry (e.g. the first frame).


## Output Format

NRRD (.nrrd) or other formats supported by MITK

The output is a single 4D image (3D+t) combining the input volumes.