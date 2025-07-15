
# MITKFileConverter User Guide {#MITKFileConverterPage}

[TOC]

## Overview

**MITKFileConverter** is a flexible command-line utility for converting medical image data between file formats using the Medical Imaging Interaction Toolkit (MITK). It supports advanced input/output customization using JSON-formatted options, provides control over reader selection, and enables handling of multiple outputs and split volumes.

MITKFileConverter is designed for:

- Converting between supported file formats (e.g., DICOM → NRRD, NRRD → NIfTI).
- Enforcing a specific reader when loading a file.
- Passing custom reader/writer options via JSON.
- Listing all available reader types for a given input.
- Handling multi-object loads (e.g., DICOM folder with multiple series).
- Providing detailed output for debugging and split detection (e.g., missing slices).

### Supported Input/Output Formats

The supported formats depend on the MITK build configuration and plugins, but typically include:

- **Input formats:** `.nrrd`, `.nii`, `.nii.gz`, `.dcm`, `.ima`, `.mhd`, `.mha`, etc.
- **Output formats:** `.nrrd`, `.nii`, `.nii.gz`, `.mhd`, `.mha`, etc.

## Usage

```bash
MITKFileConverter -i <input_file> -o <output_file> [options]
```

### Required Arguments

| Argument       | Short | Description                                                                 |
|----------------|-------|-----------------------------------------------------------------------------|
| `--input`      | `-i`  | Input file or path. May be a single file or a directory (e.g., DICOM folder). |
| `--output`     | `-o`  | Output file path. For multiple outputs, index suffixes are added automatically. |

### Optional Arguments

| Argument             | Short | Description                                                                 |
|----------------------|-------|-----------------------------------------------------------------------------|
| `--reader`           | `-r`  | Force a specific reader plugin (by name).                                   |
| `--list-readers`     | `-lr` | Print a list of all available readers for the given input.                  |
| `--input-options`    |       | JSON dictionary string containing reader-specific options (as key/value pairs).        |
| `--output-options`   |       | JSON dictionary string containing writer-specific options (as key/value pairs).        |
| `--help`             | `-h`  | Show help text.                                                             |

## Examples

### Example 1: Convert a DICOM folder to NRRD

```bash
MITKFileConverter -i ./dicom_series/ -o output_image.nrrd
```

- **What happens:** DICOM folder is read and converted to one or multiple NRRD files.
- **Readers used:** Automatically selected based on MITK reader priority.
- **Output:** `output_image.nrrd` if one object; additional files like `output_image_1.nrrd` if multiple series/time points are present.

---

### Example 2: Enforce reader and set reader options (e.g., for DICOM)

```bash
MITKFileConverter -i ./dicom_series/ -o image.nrrd -r "CUSTOM_DICOM_READER" --input-options '{"FancyOption": true}'
```

- **What happens:** DICOM reader "CUSTOM_DICOM_READER" is forced.
- **Options used:** `{"FancyOption": true}`
- **Output:** One or more `.nrrd` files, named `image.nrrd`, `image_1.nrrd`, etc.

---

### Example 3: Convert NRRD to NIfTI with compression

```bash
MITKFileConverter -i input_image.nrrd -o output_image.nii.gz
```

- **What happens:** Input NRRD image is converted to compressed NIfTI format.
- **Writer options used:** `{"useCompression": true}`
- **Output:** `output_image.nii.gz`

---

### Example 4: List available readers for a file

```bash
MITKFileConverter -i image.nrrd --list-readers
```

- **What happens:** Lists all reader plugins registered for the given file.
- **Note:** This is a diagnostic/debugging tool; no output files are generated.

---

## Handling Multiple Outputs

When a single input results in multiple outputs (e.g., multi-frame DICOM):

- The first object will be written to `<output>.ext`
- Subsequent objects will be named as `<output>_1.ext`, `<output>_2.ext`, etc.

## Missing Slice Detection

If a volume is split due to missing slices:

- A warning will be printed.
- The tool estimates how many slices were missing.
- Converted data might be incomplete or invalid — please inspect the input and the output closely.

## JSON Option Format

Options must be passed as valid JSON strings. Example:

```bash
--input-options '{"Load4D": true, "FixOrientation": false}'
```

### Supported Value Types

- Booleans: `true`, `false`
- Integers/Floats: `42`, `3.14`
- Strings: `"some string"`

Invalid types (arrays, objects) are not supported and will cause an error.