# MitkImageTypeConverter User Guide {#MitkImageTypeConverterPage}

[TOC]

## Overview

**MitkImageTypeConverter** is a command-line utility provided by the Medical Imaging Interaction Toolkit (MITK) for converting the scalar pixel type of an image to a specified type. It supports common scalar data types such as `int`, `float`, `uchar`, etc., and works on both 2D and 3D images.

The tool is useful for standardizing image data types before further processing or analysis, especially when specific types are required by downstream tools.


## Usage

```bash
MitkImageTypeConverter -i <input_file> -o <output_file> -t <type>
```


### Required Arguments

| Argument     | Short | Type   | Description                                                 |
|--------------|-------|--------|-------------------------------------------------------------|
| `--input`    | `-i`  | File   | Path to the input image file.                              |
| `--output`   | `-o`  | File   | Path to the output image file.                             |
| `--type`     | `-t`  | String | Scalar data type to convert to. See supported types below. |


### Optional Arguments

| Argument   | Short | Description         |
|------------|-------|---------------------|
| `--help`   | `-h`  | Show help text only |

---

## Supported Scalar Data Types

The following scalar types are supported via the `--type` argument:

- `int` – signed integer
- `uint` – unsigned integer
- `short` – signed short integer
- `ushort` – unsigned short integer
- `char` – signed char
- `uchar` – unsigned char
- `float` – single-precision float
- `double` – double-precision float
- `none` – no conversion is performed

> If `none` is specified, the input is not converted and no output is produced.

If an unsupported type is given, the tool defaults to `double`.


## Examples

### Example 1: Convert an image to unsigned short

```bash
MitkImageTypeConverter -i input.nrrd -o output.nrrd -t ushort
```

**Result:**  
The input image is converted to type `unsigned short` and saved as `output.nrrd`.

---

### Example 2: Convert image to float

```bash
MitkImageTypeConverter -i input.mhd -o output.mhd -t float
```

**Result:**  
The scalar type of the image is converted to `float`.

---

### Example 3: Show help

```bash
MitkImageTypeConverter --help
```

**Result:**  
Displays usage and options information.

---

## Notes

- Only 2D and 3D images are supported.
- If an unsupported dimension is provided, the tool prints a warning and exits without producing output.
- All MITK-supported file formats can be used as input/output (e.g., `.nrrd`, `.nii`, `.mhd`, `.mha`).
- The internal conversion uses ITK image casting; ensure the data remains valid after casting, especially for truncated types (e.g., `char`, `uchar`).

