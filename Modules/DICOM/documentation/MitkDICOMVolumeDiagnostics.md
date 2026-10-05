# MitkDICOMVolumeDiagnostics {#MitkDICOMVolumeDiagnosticsPage}

[TOC]

## Overview

MitkDICOMVolumeDiagnostics reports how the MITK DICOM reader would turn a set of DICOM files into image volumes: which files are analyzed, which reader configurations are tried, which one is selected, how many volumes result, which files and how many time steps each volume contains, why a volume was split, and what is worth knowing about the multi-frame files among them. The report is printed as JSON and can optionally be written to a file.

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

For every volume, the report lists its files in slice order, the number of time steps, the number of reader entries per time step (see below), and, if the reader had to split it off, the split reasons.

### Multi-frame files

The app analyzes the files a second time with the DCMTK-based scanner before reporting, because that is what the reader does before loading and only that scanner can look into sequences. Without it the report would show no frame model at all. The second scan roughly doubles the analysis time, which is accepted for a diagnostics tool.

A volume's `frame_model` is a per-volume flag: it is `true` if any file that makes up the volume has a frame model, even if the others do not.

A volume is described in reader entries. A file contributes one entry, except a file with a frame model, that is, a Per-Frame Functional Groups Sequence with one item per frame, which contributes one entry per frame. `files` lists the entries in slice order and `frames_per_timesteps` counts them per time step. So for a 20-frame single-file volume with a frame model, `files` repeats the one filename 20 times, `distinct_files` has one entry, and `frames_per_timesteps` is 20 where it was 1 before the frame model existed. A multi-frame file without a frame model, such as an RT Dose, still contributes one entry whatever its number of frames, so `frames_per_timesteps` is 1 for it although it loads as many slices. The number of frames a file holds, attribute Number of Frames (0028,0008), is reported as `number_of_frames` in the findings.

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
      "distinct_files": ["/data/patient123/IM0001.dcm", "/data/patient123/IM0002.dcm"],
      "frame_model": false,
      "volume_split_reason": [["missing_slices", "2"], ["slice_distance_inconsistency", "3.0"]]
    }
  ],
  "findings": [],
  "findings_summary": { "warning": 0, "info": 0 }
}
```

`volume_split_reason` is only present if the volume has at least one split reason. It is an array of arrays; each inner array holds the reason type and, if available, a detail string. Possible reason types are `value_split_difference`, `value_sort_distance`, `image_position_missing`, `overlapping_slices`, `gantry_tilt_difference`, `slice_distance_inconsistency`, `missing_slices`, `multi_frame_file_separated`, and `unknown`. For `missing_slices` the detail is the estimated number of missing slices, for `slice_distance_inconsistency` the detected inconsistency value, and for `multi_frame_file_separated` the number of volumes the block became.

`multi_frame_file_separated` is the expected path, not an error: a file with the per-frame read model and more than one frame cannot share a volume with another file, so each one gets a volume of its own and loads completely. The remaining files of the block, including single-frame files with the per-frame read model, keep one volume together. Every volume the block became carries the reason, the one of the remaining files as well.

The `frame_count_mismatch` reason exists but cannot appear here, because it is raised while pixel data is read and this tool does not load images.

### Findings

`findings` lists what the multi-frame analysis noticed, and `findings_summary` counts them by severity so a script can triage without walking the array. Both are always present; `findings` is an empty array when there is nothing to report. The analysis runs per file, so a volume made of several files can contribute a finding for each of them. Each entry has a stable snake_case `type`, a `severity`, a human-readable `message`, the `volume_index` it belongs to, the `files` it was found in, and a `details` object carrying only the counts that apply. The `message` may be reworded between releases; the `type` and `severity` keys are the machine-readable contract. For a volume made of a single RT Dose file, for example:

```json
"findings": [
  {
    "type": "no_per_frame_metadata",
    "severity": "info",
    "message": "Multi-frame object without per-frame functional groups; per-frame values are not available.",
    "volume_index": 0,
    "files": ["/data/patient123/RD.dcm"],
    "details": { "number_of_frames": 263 }
  }
],
"findings_summary": { "warning": 0, "info": 1 }
```

| `type` | `severity` | Meaning | `details` |
|--------|------------|---------|-----------|
| `no_per_frame_metadata` | `info` | More than one frame and no per-frame functional groups at all. The normal state of RT Dose, multi-frame NM, SC and US: the frames load as slices, but no per-frame value is available. | `number_of_frames` |
| `ragged_functional_groups` | `warning` | The Per-Frame Functional Groups Sequence has items, but not one per frame, so its values cannot be mapped to slices. The file is read as a single frame and its functional-group values are not published. | `number_of_frames`, `per_frame_item_count` |
| `varying_per_frame_rescale` | `info` | The Pixel Value Transformation differs between frames. The reader applies each frame's own pair; reported because the pixel values of such a file differ from what a reader without the per-frame model produces. | `number_of_frames`, `per_frame_item_count`, `distinct_rescale_pairs` |
| `shared_and_per_frame_rescale` | `warning` | A shared and a per-frame Pixel Value Transformation are both present, which is not conformant. The per-frame one is used as the more specific. | `number_of_frames`, `per_frame_item_count` |

There is no `error` severity. Nothing the multi-frame analysis detects stops a volume from loading.

### Exit code

The app exits with 0 after printing the report, including when the report contains a missing-slice warning or findings of any severity. The app reports; it does not adjudicate. It exits with 1 if no arguments are given (the help text is printed instead), if no DICOM files are found, if a file input cannot be matched to a file in that directory listing, or if no reader configuration can handle the files.

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
