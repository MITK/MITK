# MitkDICOMVolumeDiagnostics {#MitkDICOMVolumeDiagnosticsPage}

[TOC]

## Overview

MitkDICOMVolumeDiagnostics reports how the MITK DICOM reader would turn a set of DICOM files into image volumes: which files are analyzed, which reader configurations are tried, which one is selected, how many volumes result, which files and time steps each volume contains, and why a volume was split. The report is printed as JSON and can optionally be written to a file.

Use it when a DICOM series loads as several volumes, as a dynamic image although you expected a static one (or vice versa), or with a missing-slice warning, and you want to see the reason before converting the data with [MitkFileConverter](@ref MITKFileConverterPage). No image data is loaded and nothing is written except the optional report file.

## Usage

```bash
MitkDICOMVolumeDiagnostics -i <input> [-o <report.json>] [-s] [-d] [-t]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | A DICOM file or a directory containing DICOM files. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--output` | `-o` | File | | Path of a file to which the report is written as JSON. |
| `--only-own-series` | `-s` | Flag | | If the input is a file, analyze only the files in its directory that have the same Series Instance UID. Has no effect for directory inputs. |
| `--check-3d` | `-d` | Flag | | Analyze the input with the built-in 3D reader configurations. |
| `--check-3d+t` | `-t` | Flag | | Analyze the input with the built-in 3D+t (dynamic image) reader configurations. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

If neither `--check-3d` nor `--check-3d+t` is given, both sets of configurations are used. If only one of them is given, only that set is used.

## Details

### Which files are analyzed

- Directory input: all DICOM files directly in the directory (not recursive).
- File input: all DICOM files in the directory of the file. With `--only-own-series` this is narrowed down to the files that share the Series Instance UID of the input file.

Files that are not DICOM files are ignored. If no DICOM file is found, the app aborts with an error.

### Reader selection

The analyzed files are offered to every loaded built-in reader configuration (3D, 3D+t, or both, see above). All configurations are listed under `checked_readers`. The first configuration that produces the minimum number of output volumes is selected and listed under `selected_reader`, including its full configuration dump in `config_details`. This mirrors the selection the MITK DICOM reader performs when it loads the files.

### Which volumes are reported

For a directory input all volumes produced by the selected reader are reported. For a file input only the volumes that contain the input file are reported, because these are the volumes the MITK DICOM reader would load for that file. `volume_count` counts the reported volumes only.

For every volume, the report lists its files in slice order, the number of time steps, the number of frames per time step, and, if the reader had to split it off, the split reasons.

### Missing slice warning

If any reported volume was split because of missing slices, a warning with the summed estimated number of missing slices is printed to the console after the JSON report:

```
!!! WARNING: MISSING SLICES !!!
Details: Reader indicated volume splitting due to missing slices. Converted data might be invalid/incomplete.
Estimated number of missing slices: 2
```

The warning is not part of the JSON; the underlying information is available in the `volume_split_reason` entries.

### Report format

The report is printed to standard output with an indentation of two spaces. The file written with `--output` contains the same JSON without line breaks. Keys:

```json
{
  "input": "/data/patient123/IM0001.dcm",
  "only-own-series": true,
  "check-3d": true,
  "check-3d+t": true,
  "analyzed_files": ["/data/patient123/IM0001.dcm", "/data/patient123/IM0002.dcm"],
  "checked_readers": [
    {
      "class_name": "DICOMITKSeriesGDCMReader",
      "configuration_label": "Image Position",
      "configuration_description": "..."
    }
  ],
  "selected_reader": {
    "class_name": "DICOMITKSeriesGDCMReader",
    "configuration_label": "Image Position",
    "configuration_description": "...",
    "config_details": "..."
  },
  "volume_count": 1,
  "volumes": [
    {
      "files": ["/data/patient123/IM0001.dcm", "/data/patient123/IM0002.dcm"],
      "timesteps": 1,
      "frames_per_timesteps": 2,
      "volume_split_reason": [["missing_slices", "2"], ["slice_distance_inconsistency", "3.0"]]
    }
  ]
}
```

`volume_split_reason` is only present if the volume has at least one split reason. It is an array of arrays; each inner array holds the reason type and, if available, a detail string. Possible reason types are `value_split_difference`, `value_sort_distance`, `image_position_missing`, `overlapping_slices`, `gantry_tilt_difference`, `slice_distance_inconsistency`, `missing_slices`, and `unknown`. For `missing_slices` the detail is the estimated number of missing slices, for `slice_distance_inconsistency` the detected inconsistency value.

### Exit code

The app exits with 0 after printing the report, including when the report contains a missing-slice warning. It exits with 1 if no arguments are given (the help text is printed instead), if no DICOM files are found, or if no reader configuration can handle the files.

## Examples

### Analyze a directory

```bash
MitkDICOMVolumeDiagnostics -i /data/patient123/ct_series
```

Analyzes all DICOM files in the directory with the 3D and 3D+t configurations and prints the report to the console.

### Analyze the series of one file and save the report

```bash
MitkDICOMVolumeDiagnostics -i /data/patient123/IM0001.dcm -s -o ct_diagnostics.json
```

Only the files in the directory of `IM0001.dcm` that belong to its series are analyzed, and only the volumes containing `IM0001.dcm` are reported. The report is printed and written to `ct_diagnostics.json`.

### Check how a dynamic series would be interpreted as static 3D volumes

```bash
MitkDICOMVolumeDiagnostics -i /data/patient123/dce_mri -d
```

Restricts the analysis to the 3D configurations, so the report shows into how many separate 3D volumes the dynamic series would be split when the 3D+t configurations are not available.
