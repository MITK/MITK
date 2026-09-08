# MitkCLGlobalImageFeatures {#MitkCLGlobalImageFeaturesPage}

[TOC]

## Overview

MitkCLGlobalImageFeatures computes global (radiomics-style) image features for an image and a mask: shape and volume descriptors, first order and histogram statistics, intensity volume histogram features, local intensity features, and texture matrix features (co-occurrence, run length, size zone, distance zone, neighbouring grey level dependence, neighbourhood grey tone difference). Features are computed over the voxels where the mask is greater than 0. Every feature class has to be switched on explicitly, or all classes are computed with `--all-features`.

The input is an intensity image and a mask image with matching geometry. Results are appended as semicolon-separated rows to a text file; optionally a structured XML report of the run is written as well. Before feature computation the app can resample the mask to the image grid, resample the image to a fixed isotropic resolution, and force mismatching origins and spacings to agree. The app can process a volume as a whole or slice by slice.

Related apps: [MitkCLMRNormalization](@ref MitkCLMRNormalizationPage) and [MitkCLN4](@ref MitkCLN4Page) preprocess MR images before feature extraction, [MitkCLScreenshot](@ref MitkCLScreenshotPage) renders images and segmentations to PNG files, and [MitkResampleImage](@ref MitkResampleImagePage) and [MitkResampleMask](@ref MitkResampleMaskPage) resample data independently of this app. This app is built with the CMake option `BUILD_ClassificationCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkCLGlobalImageFeatures -i <image> -m <mask> -o <features.csv> <feature switches> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | Image | Path to the intensity image. |
| `--mask` | `-m` | Image | Path to the mask image. Voxels with a value greater than 0 define the region for which the features are computed. |
| `--output` | `-o` | File | Path to the text result file. Results are appended to this file; it is created if it does not exist. |

### Optional arguments

General options:

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--help` | `-h` | Flag | | Not a declared argument. The help text is printed by the argument parser whenever a required argument is missing, so calling the app with `-h` alone shows the help; the app then exits with code 1. |
| `--morph-mask` | `-morph` | Image | the mask | Separate mask for morphological features. Only the Grey Level Distance Zone class evaluates it; all other classes use `--mask`. |
| `--xml-output` | `-x` | File | | Additionally write the results of the run as an XML report to this path. Not supported together with `--slice-wise`. |
| `--logfile` | `-log` | File | | Append a short processing log to this file. |
| `--save-image` | `-save-image` | File | | Save the image as it is used for the analysis (after resampling and geometry correction) to this path. |
| `--save-mask` | `-save-mask` | File | | Save the mask as it is used for the analysis (after resampling and geometry correction) to this path. |
| `--save-image-screenshots` | `-save-screenshot` | File | | Render PNG screenshots of every slice of the analysed image and mask. The value is used as a path prefix, see Details. |
| `--header` | `-head` | Flag | | Write a header row with the feature names before the first result row and add the columns SoftwareVersion, Patient, Image and Segmentation. |
| `--first-line-header` | `-fl-head` | Flag | | Like `--header`, but only if the output file does not exist yet. |
| `--decimal-point` | `-decimal` | String | `.` | Character used as decimal separator in the result file and on the console. Only the first character of the value is used. |
| `--description` | `-d` | String | | Free text that is written as the first column of every result row (column Description). |
| `--direction` | `-dir` | String | `0` | Direction setting passed to all feature classes (relevant for co-occurrence and run length features). 0: all directions, 1: a single direction (for test purposes), 2, 3, 4, ...: all directions without dimension 0, 1, 2, .... Only the first value of a semicolon-separated list is used. |
| `--slice-wise` | `-slice` | String | | Process the volume slice by slice along the given axis (0, 1 or 2; any other value falls back to 0). See Details. |
| `--output-mode` | `-omode` | Int | `0` | Layout of the result file. 0: one row per image or slice, 1: one column per image or slice, 2: one row per feature. |
| `--all-features` | `-a` | Flag | | Compute every feature class regardless of the feature switches. |
| `--encode-parameter-in-name` | `-encode-parameter` | Flag | | Encode the histogram and range parameters in the feature names of the text output, for example `First Order::Min-0_Max-100_Bins-64::Mean`. |
| `--pipeline-uid` | `-p` | String | empty | Identifier of the processing pipeline. Written to the XML report only. |

Geometry options:

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--resample-mask` | `-rm` | Flag | | Resample the mask to the grid of the (possibly resampled) image using nearest neighbour interpolation. |
| `--same-space` | `-sp` | Flag | | If origin or spacing of image and mask differ, overwrite the image geometry with the mask geometry instead of aborting. |
| `--fixed-isotropic` | `-fi` | Float | | Resample the image to an isotropic resolution of the given size in mm (linear interpolation). Should be combined with `--resample-mask`. |

Global histogram options (apply to every feature class that uses a histogram, see Details):

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--minimum-intensity` | `-minimum` | Float | | Lower bound of the histogram range. |
| `--maximum-intensity` | `-maximum` | Float | | Upper bound of the histogram range. |
| `--bins` | `-bins` | Int | `256` | Number of histogram bins. |
| `--binsize` | `-binsize` | Float | | Width of a histogram bin. |
| `--ignore-mask-for-histogram` | `-ignore-mask` | Flag | | Derive the histogram range from the whole image instead of the masked region. |

Feature class switches. Every class is only computed when its switch is given (or with `--all-features`). The class name is the prefix used for the feature names in the output. Classes marked "yes" in the histogram column accept the per-class histogram options listed below the table.

| Argument | Short | Class name in output | Histogram | Class-specific options |
|----------|-------|----------------------|-----------|------------------------|
| `--volume` | `-vol` | `Volumetric Features` | no | |
| `--volume-density` | `-volden` | `Morphological Density` | no | |
| `--curvature` | `-cur` | `Curvature Feature` | no | |
| `--first-order` | `-fo` | `First Order` | yes | |
| `--first-order-numeric` | `-fon` | `First Order Numeric` | yes | |
| `--first-order-histogram` | `-foh` | `First Order Histogram` | yes | |
| `--intensity-volume-histogram` | `-ivoh` | `Intensity Volume Histogram` | yes | |
| `--local-intensity` | `-loci` | `Local Intensity` | no | `-loci::range <Float>`: radius of the local neighbourhood in mm (default 6.2). |
| `--deprecated-cooccurrence` | `-deprecated-cooc` | `Deprecated Co-occurrence Features` | no | `-deprecated-cooc::range <String>`: semicolon-separated list of distances (default `1`). |
| `--cooccurence2` | `-cooc2` | `Co-occurenced Based Features` | yes | `-cooc2::range <String>`: semicolon-separated list of distances (default `1`). |
| `--neighbouring-grey-level-dependence` | `-ngld` | `Neighbouring Grey Level Dependence` | yes | `-ngld::range <String>`: semicolon-separated list of distances (default `1`); `-ngld::alpha <Int>`: dependence coarseness parameter (default 0). |
| `--run-length` | `-rl` | `Run Length` | yes | |
| `--grey-level-sizezone` | `-glsz` | `Grey Level Size Zone` | yes | |
| `--distance-zone` | `-gldz` | `Grey Level Distance Zone` | yes | Uses `--morph-mask` for the distance map if given. |
| `--image-diagnostic` | `-id` | `Diagnostic` | no | |
| `--neighbourhood-grey-tone-difference` | `-ngtd` | `Neighbourhood Grey Tone Difference` | yes | `-ngtd::range <Int>`: neighbourhood distance (default 1). |

Per-class histogram options. Replace `<short>` by the short switch of the class, for example `-fo::min 0` or `--cooc2::bins 32`. The long form uses the `--` prefix, the short form the `-` prefix.

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--<short>::minimum` | `-<short>::min` | Float | Lower bound of the histogram range for this class. |
| `--<short>::maximum` | `-<short>::max` | Float | Upper bound of the histogram range for this class. |
| `--<short>::bins` | `-<short>::bins` | Int | Number of bins for this class. |
| `--<short>::binsize` | `-<short>::binsize` | Float | Bin width for this class. |
| `--<short>::ignore-global-histogram` | `-<short>::ignore-global-histogram` | Flag | Ignore the global histogram options for this class and use only the per-class ones. |
| `--<short>::ignore-mask-for-histogram` | `-<short>::ignore-mask` | Flag | Derive the histogram range for this class from the whole image. |

## Details

### Input handling and geometry checks

Image and mask are loaded with the MITK I/O infrastructure, so every image format MITK can read is accepted. The mask is interpreted as unsigned short; every voxel greater than 0 belongs to the region of interest. Voxels of the image that are NaN are excluded from the region through an internal copy of the mask; this copy is used by the feature classes that evaluate intensities.

If image and mask have different dimensions, a 2D image or a 2D mask is converted to a 3D volume with one slice. Afterwards the following steps are applied in this order:

1. `--fixed-isotropic`: the image is resampled to the given isotropic spacing with linear interpolation. The number of voxels per axis becomes `size * spacing / resolution + 1`.
2. `--resample-mask`: the mask is resampled onto the grid of the image with nearest neighbour interpolation.
3. Origin check: if the origins differ, the app aborts with exit code -1 unless `--same-space` is set. With `--same-space` the image origin is overwritten with the mask origin.
4. Spacing check: if the spacings differ, the app aborts with exit code -1 unless `--same-space` is set. With `--same-space` the image spacing is overwritten with the mask spacing. The messages printed in this case state the opposite (that the mask was adapted); the code adapts the image.

Image and mask are expected to be 3D volumes. 3D+t data is not handled separately; several processing steps cast the data to 3D ITK images and fail for other dimensions. Split a 3D+t image into 3D volumes first, for example with [MitkSplit4Dto3DImages](@ref MitkSplit4Dto3DImagePage).

### Feature selection

The feature classes are computed in the order of the table above. A class is computed only if its switch is present or `--all-features` is given. Without any switch and without `--all-features`, the result rows contain no feature values.

### Histogram configuration

Feature classes marked "yes" in the histogram column discretise the intensities before computing features. The histogram is configured from the global options and from the per-class options; a per-class option overrides the corresponding global one. `--<short>::ignore-global-histogram` discards all global settings for that class.

The effective settings select the initialisation strategy:

- minimum, maximum and bin size: the range is fixed and the number of bins follows from the bin size.
- minimum, bin count and bin size: the range starts at the minimum and spans `bins * binsize`.
- minimum, maximum and bin count: the range is fixed and divided into the given number of bins.
- bin size only (optionally with either minimum or maximum): the missing bound is taken from the image, from the masked region by default or from the whole image with `--ignore-mask-for-histogram`.
- bin count only: the range is taken from the whole image. In this branch a minimum or maximum that is given without the other bound is ignored, and the mask is not considered even without `--ignore-mask-for-histogram`.

Since a bin count of 256 is in effect by default, the last case applies when no histogram option is given at all: 256 bins over the intensity range of the whole image.

With `--encode-parameter-in-name` the effective settings are encoded in the feature names of the text output (for example `Min-0_Max-100_Bins-64` or `Bins-256_FullImage`, followed by `_Range-<n>` for the classes that have a range parameter). The XML report always stores the settings per feature, independent of this flag.

### Text output

The result file is opened in append mode and written when the app finishes. Values are separated by `;`. With `--output-mode 0` (the default) every processed image or slice produces one row:

```
<MITK revision>;<image folder>;<image name>;<mask name>;[<description>;]<slice number>;<value 1>;<value 2>;...;EndOfMeasurement;
```

The Description column is present only when `--description` is given. The slice number is 0 when the whole volume is processed. With `--header` (or `--first-line-header` on a new file) a header row with the column names `SoftwareVersion;Patient;Image;Segmentation;[Description;]SliceNumber;<feature names>;EndOfMeasurement;` precedes the first result row. `--output-mode 1` transposes this layout so that every image or slice becomes one column. `--output-mode 2` writes one row per feature in the form `[<description>;]<slice number>;<MITK revision>;<image folder>;<image name>;<mask name>;<feature name>;<value>;;`. The help text describes mode 2 as an XML report, but the XML report is only produced by `--xml-output`.

Feature names in the text output follow the pattern `<class name>::<feature name>`, for example `First Order::Mean`. Every computed feature is also echoed to the console as `<feature name> - <value>`. `--decimal-point` affects both the file and the console output.

### Slice-wise processing

With `--slice-wise <axis>` the image, the mask, the NaN-free mask and the morphological mask are cut into 2D slices perpendicular to the given axis (0, 1 or 2 in index space). Slices without any mask voxel are skipped. Every remaining slice is processed like an independent image and produces its own result row; the slice number column counts the processed slices starting at 0, not the index in the volume. After the last slice two additional rows are appended: the mean over all slices and the population variance over all slices of every feature, with feature names prefixed by `SliceWise Mean` and `SliceWise Var.` and with the total number of processed slices as slice number. In the Segmentation column these two rows carry the mask name with the suffixes ` - Mean` and ` - Var.`.

`--save-image`, `--save-mask` and `--save-image-screenshots` are executed per slice. The first two write to the same path for every slice, so the file contains the last processed slice. `--xml-output` is refused in slice-wise mode: the text output is written, an error is printed and the app returns exit code 1.

`--slice-wise` is silently ignored for 2D input.

### XML report

`--xml-output` writes a report in the `https://www.mitk.org/Phenotyping` namespace that contains the method name (`CLGlobalImageFeatures`), its version, the organisation, the generation date and time, the pipeline UID, all command line arguments, metadata of image and mask (file path, reader information and, for DICOM input, series and SOP instance UIDs), and every feature with name, feature class, version, setting identifier and value. The feature settings (parameters per setting identifier) are listed separately. Numbers in the XML report always use `.` as decimal separator.

### Screenshots

`--save-image-screenshots <prefix>` renders every slice of the analysed image with the mask overlaid to a PNG file named `<prefix>_Idx-<n>_Step-<s>.png`, where `n` is the slice number of the processed image (0 for the whole volume) and `s` the slice position of the rendered view. The directory part of the prefix is created if necessary; if the prefix ends with a path separator, the files are placed in that directory. Screenshots are 768 x 768 pixels (a 256 x 256 render window magnified three times). The file name is read back through a stream, so prefixes containing whitespace are truncated at the first space.

The app creates a Qt application object unconditionally, even without screenshots, so a Qt platform (a display or an offscreen platform plugin) must be available.

### Log file

`--logfile` appends the version string, the image and mask paths and one marker per processing step to the given file, without line breaks between the markers. The log is only useful to see how far a run got.

## Exit codes

| Code | Meaning |
|------|---------|
| 0 | Success. |
| 1 | No arguments or missing required arguments (help text is printed), or `--xml-output` combined with `--slice-wise` (text output is still written). |
| -1 | Origin or spacing of image and mask differ and `--same-space` was not given. |

Runtime errors such as unreadable files raise exceptions that are not caught by the app; the process terminates with the platform-specific code for an unhandled exception.

## Examples

### First order and shape features with a header row

```bash
MitkCLGlobalImageFeatures -i ct.nrrd -m mask.nrrd -o features.csv -fo -vol -head
```

Computes the First Order and Volumetric Features classes for the masked region and appends a header row and one result row to `features.csv`. The histogram of the first order features uses 256 bins over the intensity range of the whole image.

### All features with a fixed histogram and an XML report

```bash
MitkCLGlobalImageFeatures -i ct.nrrd -m mask.nrrd -o features.csv -a -minimum -1000 -maximum 1000 -bins 64 -x features.xml -p "study-42"
```

Computes every feature class. All histogram-based classes discretise the intensities into 64 bins between -1000 and 1000. Besides the text row, `features.xml` receives the full report including the pipeline UID `study-42`.

### Slice-wise texture features on resampled data

```bash
MitkCLGlobalImageFeatures -i mr.nrrd -m tumor.nrrd -o slices.csv -fi 1.0 -rm -sp -slice 2 -cooc2 -rl -fl-head
```

Resamples the image to 1 mm isotropic voxels, resamples the mask onto that grid, forces origin and spacing to agree, and computes co-occurrence and run length features for every axial slice (axis 2) that contains mask voxels. One row per slice plus a mean and a variance row are appended; the header row is written only if `slices.csv` does not exist yet.

### Per-class settings and one row per feature

```bash
MitkCLGlobalImageFeatures -i ct.nrrd -m mask.nrrd -o long.csv -fo -fo::bins 32 -cooc2 -cooc2::range "1;2;4" -encode-parameter -d lesion-1 -omode 2
```

Uses 32 bins for the First Order class only, computes the co-occurrence features for the distances 1, 2 and 4, encodes these settings in the feature names, and writes one row per feature with the description `lesion-1` in the first column.
