# MitkMatchImage User Guide {#MitkMatchImagePage}

[TOC]

## Overview

**MitkMatchImage** is a lightweight, command-line image registration tool built using the **MITK** (Medical Imaging Interaction Toolkit) and **MatchPoint** libraries. This tool enables users to register a moving image to a target image using a specified registration algorithm and supports advanced configurations like masking and parameter tuning.
It is often used together with the application MitkMapImage which allows applying a registration to an image and map it accordingly into the target space.

**Features**

- Register images using a plugin-based algorithm.
- Apply optional image masks for targeted registration.
- Accepts custom algorithm parameters via JSON input.
- Supports multi-label image segmentation.
- Outputs registration results as a file.

## Usage

```bash
MitkMatchImage -m <movingImage> -t <targetImage> -a <algorithmDLL> -o <outputFile> [optional parameters]
```

### Required Arguments

| Argument | Short | Description |
|----------|-------|-------------|
| `--moving` | `-m` | Path to the moving image file. |
| `--target` | `-t` | Path to the target image file. |
| `--algorithm` | `-a` | Path to the deployed registration algorithm DLL. |
| `--output` | `-o` | Output file for the computed registration result. |

### Optional Arguments

| Argument | Short | Description |
|----------|-------|-------------|
| `--moving_mask` | `-mm` | Mask for the moving image. |
| `--moving_mask_label` | `-mml` | Label name to extract from the moving mask if it is a MITK Segmentation with multiple labels. |
| `--target_mask` | `-tm` | Mask for the target image. |
| `--target_mask_label` | `-tml` | Label name to extract from the target mask if it is a MITK Segmentation with multiple labels. |
| `--parameters` | `-p` | JSON string of algorithm parameters. |
| `--help` | `-h` | Show help text. |

## Example

```bash
MitkMatchImage   -m lungCT_moving.nii.gz   -t lungCT_target.nii.gz   -a rigidReg.dll   -o outputReg.mapr   -mm movingSeg.nrrd   -mml "lung"   -tm targetSeg.nrrd   -tml "lung"   -p '{"maxIterations": 200, "tolerance": 1e-6}'
```

**What happens**:  
Registred the moving image `lungCT_moving.nii.gz` onto the target image `lungCT_target.nii.gz` by using the registration algorithm provided by `rigidReg.dll`. The algorithm will use a moving segmentation `movingSeg.nrrd` and a target segmentation `targetSeg.nrrd` to guide the registration. Both segmentations are supposed to be multi-label, so the label `lung` is selected for both. The resulting registration is stored in `outputReg.mapr`. The registration algorithm is also configured by passing some explicit parameter settings.

## Notes

- **DLL Compatibility:** Ensure the algorithm DLL is compiled with MatchPoint deployment support.
- **Masks:** When using multi-label segmentations, specify a label name to extract the right region.
- **Time Series:** Only the first time step of a 4D image is considered.
