# MitkDICOMVolumeDiagnostics User Guide {#MitkDICOMVolumeDiagnosticsPage}

[TOC]

## Overview

`MitkDICOMVolumeDiagnostics` is a command-line tool provided by MITK to analyze how a set of DICOM files would be interpreted and grouped into image volumes using MITK’s internal DICOM file reader mechanisms.

It reports the number of generated volumes, sorting behavior, volume splitting reasons, and reader configurations used to interpret the data. The diagnostic results are printed to standard output in JSON format and optionally written to a file.

This tool is useful for debugging DICOM series loading behavior and understanding why a particular DICOM dataset might result in multiple volumes or unexpected volume splits.

## Usage

```sh
MitkDICOMVolumeDiagnostics -i <input> [options]
```

### Required Arguments

| Argument                  | Short | Type   | Description                                                                                     |
|---------------------------|-------|--------|-------------------------------------------------------------------------------------------------|
| `--input`                 | `-i`  | File   | Required. A DICOM file or directory containing DICOM files.                                     |
| `--output`                | `-o`  | File   | Optional. Path to a JSON file where the diagnostic result will be saved.                       |

### Optional Arguments

| Argument                  | Short | Type   | Description                                                                                     |
|---------------------------|-------|--------|-------------------------------------------------------------------------------------------------|
| `--only-own-series`       | `-s`  | Flag   | If set and input is a file, only files from the same Series Instance UID are analyzed.         |
| `--check-3d`              | `-d`  | Flag   | Use only 3D volume configurations for diagnostics.                                              |
| `--check-3d+t`            | `-t`  | Flag   | Use only 3D+t (dynamic) volume configurations for diagnostics.                                  |
| `--help`                  | `-h`  | Flag   | Show help text.                                                                                 |

If neither `--check-3d` nor `--check-3d+t` are set, both are enabled by default.

## Behavior

1. **Input Resolution**:
   - If a directory is provided, all DICOM files in the folder are considered.
   - If a single file is provided:
     - All DICOM files in the same folder are considered.
     - If `--only-own-series` is set, only files from the same Series UID are used.

2. **Reader Configuration**:
   - MITK internally maintains a set of built-in reader configurations.
   - Depending on the `--check-3d` and `--check-3d+t` flags, 3D and/or dynamic reader configurations are loaded.
   - All configured readers are listed in the diagnostics output.
   - The best reader (producing the fewest output images) is selected for deeper analysis.

3. **Volume Output Analysis**:
   - For each output volume, the tool reports:
     - Files included
     - Number of time steps
     - Frames per time step
     - Volume splitting reasons (e.g., missing slices)
   - The tool counts and reports the number of volumes that are relevant to the input.

4. **Missing Slice Warning**:
   - If any volume was split due to missing slices, a warning message is printed and included in the diagnostics.

5. **Output**:
   - A structured JSON report is printed to the console.
   - If `--output` is specified, the same JSON report is written to the given file.

## Output JSON Structure

Example output structure:

```json
{
  "input": "path/to/input",
  "only-own-series": true,
  "check-3d": true,
  "check-3d+t": false,
  "analyzed_files": [ "file1.dcm", "file2.dcm", ... ],
  "checked_readers": [
    {
      "class_name": "ExampleReaderClass",
      "configuration_label": "3D Default",
      "configuration_description": "Standard 3D reader"
    },
    ...
  ],
  "selected_reader": {
    "class_name": "ChosenReader",
    "configuration_label": "Best Match",
    "configuration_description": "...",
    "config_details": "Detailed configuration printed as string"
  },
  "volume_count": 1,
  "volumes": [
    {
      "files": [ "file1.dcm", "file2.dcm", ... ],
      "timesteps": 1,
      "frames_per_timesteps": 20,
      "volume_split_reason": {
        "MissingSlices": "2"
      }
    }
  ]
}
```

## Notes

- The tool performs no actual image loading. It simulates how the MITK DICOM reader would interpret the input data.
- No segmentation or rendering is performed.
- A warning is shown if volumes were split due to missing slices, as this might indicate an incomplete dataset or issues with image acquisition.

## Example

```sh
MitkDICOMVolumeDiagnostics -i /data/patient123 -o diagnostics.json --only-own-series --check-3d
```

This command analyzes the DICOM files in `/data/patient123`, using only those from the same series as the input file, checks only 3D configurations, and writes the results to `diagnostics.json`.