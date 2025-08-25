
# MitkConvertToMultiLabelSegmentation User Guide {#MitkConvertToMultiLabelSegmentationPage}

[TOC]

## Overview
This command-line tool converts various medical data types (images, surfaces, and contours) into a **multi-label segmentation** using the [Medical Imaging Interaction Toolkit (MITK)](https://www.mitk.org/).

**Features**

- Converts **images**, **surface meshes**, and **contours** into a single **multi-label segmentation** output.
- Supports **grouping** inputs into distinct label groups.
- Automatically resolves conflicting label values.
- Ensures geometry consistency and validates spatial alignment.
- Accepts DICOM, NIfTI, and other MITK-compatible image formats.


## Basic Usage

```bash
convertToMultiLabelSegmentation -i <input1> <input2> ... -o <output.nrrd> [options]
```


## Required Arguments

| Argument               | Description                                                 |
|------------------------|-------------------------------------------------------------|
| `-i`, `--inputs`       | One or more input files (images, surfaces, or contours).    |
| `-o`, `--output`       | Output file path for the resulting multi-label segmentation.|

## Optional Arguments

| Argument                | Description                                                                 |
|-------------------------|-----------------------------------------------------------------------------|
| `-r`, `--reference`     | Reference image for defining the output geometry (required if no image inputs are used). |
| `-g`, `--groups`        | Enable grouping: each input is placed into a separate label group. Default: all inputs are in one group. |
| `-h`, `--help`          | Show help and usage information.                                            |


## 🧪 Examples

### Example 1: Convert a single binary image mask

```bash
convertToMultiLabelSegmentation -i tumor.nii.gz -o tumor.nrrd
```

**What happens**:  
A single binary image is converted to a multi-label segmentation with one label.

**Label & Group Naming**:  
- Group: _Unnamed default group_
- Label: `tumor Value 1`

**Resulting Pixel Values**:  
- Label `tumor Value 1`: value `1`

---

### Example 2: Merge multiple binary masks into one group

```bash
convertToMultiLabelSegmentation -i mask1.nii.gz mask2.nii.gz -o merged.nrrd
```

**What happens**:  
Two binary image masks are merged into a single label group.

**Label & Group Naming**:  
- Group: _Unnamed default group_
- Labels:
  - `mask1 Value 1`
  - `mask2 Value 1`

**Resulting Pixel Values**:  
- Label `mask1 Value 1`: value `1`  
- Label `mask2 Value 1`: value `2`

---

### Example 3: Group surface inputs into separate label groups (with reference image)

```bash
convertToMultiLabelSegmentation -i liver.stl spleen.stl kidney.stl -r reference.nii.gz -o organs.nrrd -g
```

**What happens**:  
Each surface is rasterized using the reference image and placed in its own group.

**Label & Group Naming**:  
- Group `liver`: Label `liver`  
- Group `spleen`: Label `spleen`  
- Group `kidney`: Label `kidney`

**Resulting Pixel Values**:  
- Group `liver`, Label `liver`: value `1`
- Group `spleen`, Label `spleen`: value `2`  
- Group `kidney`, Label `kidney`: value `3`

---

### Example 4: Convert a contour set using reference image

```bash
convertToMultiLabelSegmentation -i roi.contour -r referenceCT.nii.gz -o contour.nrrd
```

**What happens**:  
Contours are rasterized into a binary mask using the reference image geometry.

**Label & Group Naming**:  
- Group: _Unnamed default group_
- Label: `roi`

**Resulting Pixel Values**:  
- Label `roi`: value `1`

---

### Example 5: Input image with multiple pixel values (multi-label image)

```bash
convertToMultiLabelSegmentation -i MultiLabelSeg.nii.gz -o resegmented.nrrd
```

**What happens**:  
An image containing multiple pixel values (e.g., 1 = liver, 2 = spleen, 3 = kidney) is converted.

**Label & Group Naming**:  
- Group: _Unnamed default group_  
- Labels:
  - `MultiLabelSeg Value 1`  
  - `MultiLabelSeg Value 2`  
  - `MultiLabelSeg Value 3`

**Resulting Pixel Values**:  
- `MultiLabelSeg Value 1`: value `1`  
- `MultiLabelSeg Value 2`: value `2`  
- `MultiLabelSeg Value 3`: value `3`

---

## 📂 Details

### 🏷 Label Naming Strategy

- **Image inputs**:
  - Each unique pixel value is treated as a separate label.
  - Label names follow the format: `<filename> Value <pixel_value>`.

- **Non-image inputs** (surfaces, contours):
  - Treated as binary masks.
  - Label name is the filename without extension.

### 📁 Group Naming Strategy

- When `--groups` is **enabled**:
  - Each input becomes a separate group, named from the input filename.

- When `--groups` is **not enabled** (default):
  - All labels are placed in a single unnamed group.

### 🎯 Label Value Assignment Strategy

- **Image Inputs**:
  - Unique pixel values are extracted.
  - If label values overlap across inputs, the tool automatically remaps values to avoid conflicts.
  - Label names still reflect original values (e.g., `Value 2`), even if actual pixel values are remapped.

- **Non-Image Inputs**:
  - Converted into binary masks.
  - Default pixel value is `1`.
  - If conflicts exist, remapping is applied.

- Label values after conversion are unique over all groups.

## Output Format

The resulting file is a standard **multi-label segmentation** in NRRD format:

- Can be loaded into MITK or compatible toolkits.
- Encodes labels and groups as distinct entities.
- Stores label metadata (names, values, group names).
