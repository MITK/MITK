# MitkFileConverter {#MITKFileConverterPage}

[TOC]

## Overview

MitkFileConverter converts data between the file formats that MITK can read and write. It loads the input with the regular MITK I/O infrastructure (the same readers and writers the Workbench uses) and saves every loaded data object to the output path, so it is not limited to images: anything MITK has a reader and a writer for (images, segmentations, surfaces, point sets, ...) can be converted. The output format is determined by the file extension of the output path.

Beyond plain conversion, the app lets you inspect which readers are available for an input, enforce a specific reader, and pass reader or writer options as JSON. Typical use cases are converting a DICOM series to NRRD or NIfTI, exporting a multi-label segmentation as a stack of label images (see the subpage below), and scripted batch conversions.

For changing the pixel type of an image use [MitkImageTypeConverter](@ref MitkImageTypeConverterPage). For splitting or fusing time-resolved images use [MitkSplit4Dto3DImages](@ref MitkSplit4Dto3DImagePage) and [MitkFuse3Dto4DImage](@ref MitkFuse3Dto4DImagePage). To find out how a set of DICOM files would be grouped into volumes before converting them, use [MitkDICOMVolumeDiagnostics](@ref MitkDICOMVolumeDiagnosticsPage).

## Usage

```bash
MitkFileConverter -i <input> -o <output> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | Input path that should be loaded. May be a single file or a directory (e.g. a DICOM folder). |
| `--output` | `-o` | File | Output path where the result is stored. The extension selects the output format. If the input yields several data objects, an index suffix is added to all but the first output file (see Details). |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--reader` | `-r` | String | | Enforce a certain reader for loading the input. The value must match the reader description exactly as printed by `--list-readers`. |
| `--list-readers` | `-lr` | Flag | | Print the descriptions of all readers available for the input and exit without converting anything. |
| `--input-options` | | String | | JSON dictionary with key/value pairs that are passed as options to the selected reader. |
| `--output-options` | | String | | JSON dictionary with key/value pairs that are passed as options to the selected writer. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Output naming for multiple data objects

A single input can produce more than one data object, for example a DICOM directory that contains several series, or a series that the DICOM reader had to split into several volumes. The objects are written in the order in which they were loaded:

- the first object is written to `<output>` as given,
- every further object is written to `<name>_<n><extension>`, where `n` counts up from 1.

For `-o result.nrrd` and three loaded objects this yields `result.nrrd`, `result_1.nrrd`, and `result_2.nrrd`. Note that the built-in help text claims the index starts with 0; the app actually starts with 1.

### Reader selection

Without `--reader`, MITK selects the reader with the highest confidence for the input, as it would in the Workbench. With `--reader`, the given description is used as a preference: if a reader with exactly that description is available for the input it is used, otherwise the app silently falls back to the default selection. Use `--list-readers` to see the descriptions that are accepted for a given input.

`--list-readers` needs both `--input` and `--output` (the parser insists on all required arguments), but it does not write anything. If the input file does not exist or no reader is available, an error message is printed; the exit code is still 0 in that case.

### Reader and writer options

`--input-options` and `--output-options` take a JSON object whose keys are option names of the reader or writer and whose values are booleans, integers, floating-point numbers, or strings. Arrays and nested objects are rejected and the app exits with an error before anything is loaded. The keys must be spelled exactly like the option names the reader or writer defines (they are usually the labels shown in the option dialogs of the Workbench, e.g. `"Save strategy"`). Output options are applied to every data object that is written.

```bash
MitkFileConverter -i segmentation.nrrd -o stack.nii.gz --output-options '{"Save strategy": "instance", "Instance value": "original"}'
```

See \ref MITKCmdAppsPage for how to quote JSON strings in other shells.

### Missing slice detection

After a data object has been written, the app inspects the split-reason metadata that the MITK DICOM reader attaches to a volume it had to split. If the reader recorded missing slices, the estimated number of missing slices is summed over all outputs and a warning is printed at the end:

```
!!! WARNING: MISSING SLICES !!!
Details: Reader indicated volume splitting due to missing slices. Converted data might be invalid/incomplete.
Estimated number of missing slices: 2
```

The warning does not change the exit code. Inspect input and output closely when it appears. For a detailed report on how a DICOM series is split, use [MitkDICOMVolumeDiagnostics](@ref MitkDICOMVolumeDiagnosticsPage).

### Error handling and time-resolved data

The app tries to save every loaded object even if saving a previous one failed. The exit code is 1 if loading threw an exception, if no data object at all was loaded, or if saving any object failed; otherwise it is 0.

There is no special handling of 3D+t data: a dynamic image is loaded and written as one object, provided the writer for the chosen output format supports it.

## Examples

### Convert a DICOM series to NRRD

```bash
MitkFileConverter -i ./ct_series/ -o ct.nrrd
```

All DICOM files in `./ct_series/` are read by the MITK DICOM reader. If the folder contains a single series that forms one volume, `ct.nrrd` is written; additional volumes are written to `ct_1.nrrd`, `ct_2.nrrd`, and so on.

### List the readers available for a file

```bash
MitkFileConverter -i image.nrrd -o unused.nrrd --list-readers
```

Prints the description of every reader that can handle `image.nrrd` and exits. No file is written; `-o` only has to be present because it is a required argument.

### Enforce a reader and pass a reader option

```bash
MitkFileConverter -i legacy_labelset.nrrd -o segmentation.nrrd -r "MITK LabelSetImage (legacy)" --input-options '{"Multi layer handling": "Split layers"}'
```

The named reader is used if it is available for the input, and the option `Multi layer handling` is set to `Split layers` before loading. If the legacy segmentation contains several layers, each layer becomes its own data object and the outputs are named `segmentation.nrrd`, `segmentation_1.nrrd`, and so on.

### Convert NRRD to compressed NIfTI

```bash
MitkFileConverter -i image.nrrd -o image.nii.gz
```

The `.nii.gz` extension selects the NIfTI writer with compression; no writer options are needed.

## See also

- \subpage MITKFileConverterSegStacksPage
