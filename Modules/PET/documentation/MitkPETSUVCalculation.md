# MitkPETSUVCalculation {#MitkPETSUVCalculationPage}

[TOC]

## Overview

MitkPETSUVCalculation converts a PET image into a Standardized Uptake Value
(SUV) image. The standard input is an activity concentration image in
`[Bq/mL]`; images that already contain SUV values (`[g/mL]`, `[cm^2/mL]` or
Philips count data with private scale factors) are recognised from their DICOM
tags and re-normalized to the requested variant instead.

Five normalization variants are available: body weight (`bw`), lean body mass
after Janmahasatian (`lbm-janma`) or James (`lbm-james128`), ideal body weight
(`ibw`) and body surface area (`bsa`). The acquisition parameters (injected
dose, half-life, decay timing, patient weight, height and sex) are read from
the DICOM properties of the input by default and can be overridden
individually. The decay-correction state of the input (`(0054,1102)` Decay
Correction: `ADMIN`, `START` or `NONE`) decides how much residual decay
correction is applied, following the IBSI-SUV benchmark recommendations.

For a DICOM PET series with the standard tags nothing beyond `-i` and `-o` is
needed. Non-DICOM inputs (e.g. NRRD) need the acquisition parameters and the
validation bypass flags on the command line.

## Usage

```bash
MitkPETSUVCalculation -i <input> -o <output> [options]
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--input` | `-i` | File | PET image: a DICOM directory or file, or any image format MITK can read. |
| `--output` | `-o` | File | Path of the SUV image to write. The extension selects the format (`.nrrd` recommended). |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--variant` | | String | `bw` | Output SUV variant: `bw`, `lbm-janma`, `lbm-james128`, `ibw` or `bsa` (case-insensitive). |
| `--injected-activity` | | Float | | Injected activity in Bq. Overrides `(0018,1074)` Radionuclide Total Dose. |
| `--body-weight` | | Float | | Patient weight in kg. Overrides `(0010,1030)` Patient Weight. |
| `--patient-height` | | Float | | Patient height in m. Overrides `(0010,1020)` Patient Size. |
| `--patient-sex` | | String | | Patient sex `M`, `F` or `O` (case-insensitive). Overrides `(0010,0040)` Patient Sex. |
| `--half-life` | | Float | | Radionuclide half-life in s. Overrides `(0018,1075)` Radionuclide Half Life. |
| `--nuclide` | | String | | Radionuclide name as alternative to `--half-life`: `18F`, `68Ga`, `11C` or `15O` (case-sensitive). |
| `--decay-time` | | Float | | Uniform decay time in s applied to every voxel. Bypasses the DICOM-derived decay correction. |
| `--tracer-index` | | Int | | Zero-based index of the item to use if the Radiopharmaceutical Information Sequence `(0054,0016)` has more than one item. |
| `--ignore-modality-check` | | Flag | | Continue even if `(0008,0060)` Modality is not `PT`. |
| `--ignore-units-check` | | Flag | | Treat the pixel values as activity concentration in `[Bq/mL]` regardless of `(0054,1001)` Units. |
| `--strict-dicom` | | Flag | | Refuse the IBSI-SUV benchmark adaptations of borderline DICOM input instead of applying them with a warning. |
| `--verbose` | `-v` | Flag | | Print progress messages. |
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Normalization variants

All variants share the same kernel for activity concentration inputs:

```
SUV = pixel * scaleNumerator / (injectedActivity * 2^(-decayTime / halfLife))
```

The variant determines `scaleNumerator` and thereby the output unit
(`W` = weight in kg, `H` = height in m, `H_cm` = height in cm,
`BMI = W / H^2`):

| Variant | Scale numerator | Output unit |
|---------|-----------------|-------------|
| `bw` | `W * 1000` (Strauss and Conti 1991). | `g/mL` |
| `lbm-janma` | Janmahasatian (2005): `LBM_male = (9270 * W) / (6680 + 216 * BMI)`, `LBM_female = (9270 * W) / (8780 + 244 * BMI)`, times 1000. IBSI-SUV recommended LBM formula. | `g/mL` |
| `lbm-james128` | James (1976): `LBM_male = 1.10 * W - 0.0128 * W^2 / H^2`, `LBM_female = 1.07 * W - 0.0148 * W^2 / H^2`, times 1000. | `g/mL` |
| `ibw` | Sugawara (1999): `IBW_male = 48.0 + 1.06 * (H_cm - 152)`, `IBW_female = 45.5 + 0.91 * (H_cm - 152)`, times 1000. Not clamped below 152 cm. | `g/mL` |
| `bsa` | DuBois (1916): `BSA = 0.007184 * W^0.425 * H_cm^0.725` in m^2, times 10000. | `cm^2/mL` |

Required patient data per variant: `bw` needs the weight; `bsa` needs weight
and height; `lbm-janma`, `lbm-james128` and `ibw` need weight, height and sex
(the weight is needed by every variant because it is also the denominator of
the re-normalization path). Missing data ends the run with exit code 2 (DICOM
tag absent) or 7 (a normalization input is missing).

For the sex-specific variants a patient sex of `O` (Other) is handled with the
IBSI-SUV adaptation: the mean of the male and female scale numerators is
used, and a warning is logged. With `--strict-dicom` this is refused (exit
code 8). A DICOM sex value other than `M`, `F` or `O` (e.g. `U`) is rejected
with exit code 6.

### Input pixel semantics

The pixel semantics are classified from `(0054,1001)` Units, `(0054,1006)` SUV
Type and, for count data, `(0008,0070)` Manufacturer:

| `(0054,1001)` | `(0054,1006)` | Handling |
|---------------|---------------|----------|
| `BQML` | ignored | Activity concentration; the kernel above is applied. |
| `GML` | absent, empty or `BW` | Pre-normalized SUVbw; re-normalized to the target variant. |
| `GML` | `LBMJANMA`, `LBMJAMES128`, `IBW` | Pre-normalized SUV of that variant; re-normalized to the target variant. |
| `GML` | `LBM` | Rejected: the Morgan LBM formula is obsolete per IBSI-SUV and not supported. |
| `GML` | `BSA` | Rejected as inconsistent (BSA SUV uses `CM2ML`). |
| `CM2ML` | absent or `BSA` | Pre-normalized SUVbsa; re-normalized to the target variant. |
| `CM2ML` | other | Rejected as inconsistent. |
| `CNTS` (Philips) with SUV-scale factor | ignored | `pixel * factor` is SUVbw; re-normalized to the target variant. |
| `CNTS` (Philips) with activity-scale factor | ignored | `pixel * factor` is `[Bq/mL]`; the kernel is applied. |
| `CNTS` (Philips) without factor | any | Rejected (`MissingPhilipsPETScaleException`). |
| `CNTS` (other manufacturer) | any | Rejected (`UnsupportedPETUnitsException`). |
| other or missing | any | Rejected. |

The Philips private factors `(7053,xx00)` and `(7053,xx09)` are read by the
DICOM reader and attached as the properties `mitk.pet.PhilipsSUVScale` and
`mitk.pet.PhilipsActivityScale`. Re-normalization of pre-normalized inputs is

```
SUV_target = pixel * prenormScale * scaleNumerator(target) / scaleNumerator(source)
```

and does not consult injected activity, half-life or decay timing. Patient
data required by the source variant must be available as well.

`--ignore-units-check` skips this classification and treats the pixels as
`[Bq/mL]`. Use it only if the pixels really are activity concentrations, e.g.
for non-DICOM inputs without a Units tag.

### Acquisition parameters and overrides

Each override replaces the value of the corresponding DICOM tag. Without an
override the tag is mandatory for the pipeline that needs it; a missing tag
ends the run with exit code 2. `--injected-activity`, `--half-life`,
`--nuclide` and `--decay-time` are only consulted for activity concentration
inputs. If both `--half-life` and `--nuclide` are given, `--half-life` wins.
Weight, height and sex are entered in kg, m and `M`, `F` or `O`; the
application converts them for the formulas.

Radionuclide data (half-life, injected dose) are read from the
Radiopharmaceutical Information Sequence `(0054,0016)`. If the sequence has
more than one item, the run stops with exit code 5 unless `--tracer-index`
selects one.

### Decay correction

`(0054,1102)` Decay Correction selects how the residual decay time is
determined:

- `ADMIN`: the pixel data are already corrected to the administration time.
  The decay term is `2^0 = 1`.
- `START`: the pixel data are corrected to a scanner-specific reference time.
  The reference time is resolved with the fallback chain below.
- `NONE`: the pixel data are not decay corrected. The reference time of a
  slice is `AcquisitionDateTime + T_ave` with the average count-rate time
  `T_ave = (1/lambda) * ln((lambda * T) / (1 - exp(-lambda * T)))`,
  `lambda = ln(2) / halfLife`, `T = (0018,1242)` Actual Frame Duration. Inputs
  without `(0008,0022)`, `(0008,0032)` or `(0018,1242)` per slice are rejected
  with exit code 2; use `--decay-time` instead.

The injection time is taken from `(0018,1078)` Radiopharmaceutical Start
DateTime if present, otherwise from `(0018,1072)` Radiopharmaceutical Start
Time combined with the acquisition or series date. For the time-only tag a
negative decay time is corrected once by 24 h (injection on the previous
day). A decay time that is still negative or exceeds 24 h ends the run with
exit code 3.

The `START` fallback chain is (first applicable step wins):

1. Vendor private datetime tag: Siemens `(0071,xx22)` in private block
   "SIEMENS MED PT", GE `(0009,xx0D)` in private block "GEMS_PETD_01". The
   DICOM reader attaches them as `mitk.pet.SiemensDecayDateTime` and
   `mitk.pet.GEScanDateTime`. Used as uniform reference time for all slices.
2. `(0008,0032)` Acquisition Time equals `(0008,0031)` Series Time (second
   resolution) at slice 0 and the manufacturer is Siemens, GE or Philips: the
   per-slice Acquisition Time is the reference.
3. Siemens or Philips: per-slice reference
   `AcquisitionTime + T_ave - FrameReferenceTime`, requiring per-slice
   `(0008,0032)`, `(0054,1300)` Frame Reference Time (non-negative),
   `(0018,1242)` (positive) and a known half-life.
4. GE: per-slice reference `AcquisitionTime - FrameReferenceTime` with the
   same tag requirements as step 3.

Steps 3 and 4 are empirical vendor formulas and count as benchmark
adaptations: they are applied with a warning by default and refused with
`--strict-dicom` (exit code 8). If no step applies (typically an unknown
manufacturer without private tags and without per-slice frame timing) the run
stops with exit code 3.

`--decay-time` bypasses all of the above and applies the given duration to
every voxel. `0` reproduces `ADMIN` behaviour. Negative or non-finite values
are rejected (exit code 1). Because the value is uniform, it is not a
substitute for the per-slice handling of `START` and `NONE`.

### Modality check

By default `(0008,0060)` Modality must be `PT` (trimmed, case-insensitive).
Any other value, including a missing tag on non-DICOM input, ends the run with
exit code 4 unless `--ignore-modality-check` is set.

### Benchmark adaptations and `--strict-dicom`

The IBSI-SUV benchmark recommends a few adaptations that make real-world DICOM
input usable. By default they are applied and logged as warnings. With
`--strict-dicom` they are refused and the run stops with exit code 8:

| Adaptation | Trigger | Default | Strict |
|------------|---------|---------|--------|
| Radionuclide Total Dose `(0018,1074)` given in MBq | value strictly between `0` and `1e4` | multiplied by `1e6` | `ImplausibleRadionuclideDoseException` |
| Vendor decay-timing fallback for `START` | steps 3 or 4 of the fallback chain | applied | `VendorEmpiricalDecayFallbackRefusedException` |
| Patient sex `O` for a sex-specific variant | `(0010,0040)` or `--patient-sex` is `O` | mean of male and female numerators | `AmbiguousPatientSexAdaptationRefusedException` |

`--injected-activity` bypasses the dose adaptation and `--decay-time` bypasses
the decay-timing fallback regardless of `--strict-dicom`; both values are used
verbatim.

### Output

The output image has pixel type `double`, the geometry and time steps of the
input, and a copy of the input's properties (including the DICOM properties).
`(0054,1001)` Units and `(0054,1006)` SUV Type are set according to the target
variant; `(0028,1052)` Rescale Intercept is set to `0.0` and `(0028,1053)`
Rescale Slope to `1.0`:

| Variant | `(0054,1001)` | `(0054,1006)` |
|---------|---------------|---------------|
| `bw` | `GML` | `BW` |
| `lbm-janma` | `GML` | `LBM` |
| `lbm-james128` | `GML` | `LBM` |
| `ibw` | `GML` | `IBW` |
| `bsa` | `CM2ML` | `BSA` |

Both LBM variants share the standard SUV Type code `LBM`; record the chosen
formula elsewhere (e.g. in the file name).

## Examples

### SUVbw from a DICOM series

```bash
MitkPETSUVCalculation -i ./fdg_pet/ -o suv_bw.nrrd
```

All parameters (half-life, injected dose, decay timing, weight) are read from
the DICOM tags. The output is in `g/mL`.

### Lean body mass variant

```bash
MitkPETSUVCalculation -i ./fdg_pet/ -o suv_lbm.nrrd --variant lbm-janma
```

Additionally reads patient height and sex from DICOM. Use
`--variant lbm-james128` for the James formula instead.

### Body surface area with a manual height

```bash
MitkPETSUVCalculation -i ./fdg_pet/ -o suv_bsa.nrrd --variant bsa --patient-height 1.78
```

The dataset lacks `(0010,1020)`, so the height is supplied on the command
line. The output is in `cm^2/mL`.

### Multi-tracer dataset

```bash
MitkPETSUVCalculation -i ./multi_tracer_pet/ -o suv.nrrd --tracer-index 1
```

The Radiopharmaceutical Information Sequence has more than one item; the
second item is selected explicitly.

### NRRD input without DICOM tags

```bash
MitkPETSUVCalculation -i pet.nrrd -o suv.nrrd --injected-activity 1.85e8 --nuclide 18F --body-weight 70 --decay-time 0 --ignore-modality-check --ignore-units-check
```

All acquisition parameters come from the command line. `--decay-time 0`
corresponds to data that are already corrected to the administration time.
The modality and units checks are bypassed because NRRD carries no DICOM tags.

## Exit codes

| Code | Meaning |
|------|---------|
| `0` | Success. |
| `1` | Generic or unexpected error (including an invalid `--decay-time` value). |
| `2` | A required DICOM property is missing. |
| `3` | The decay timing is ambiguous (injection and acquisition times cannot be reconciled). |
| `4` | Invalid command-line arguments, or the modality check failed. |
| `5` | Multi-item Radiopharmaceutical Information Sequence without `--tracer-index`. |
| `6` | A DICOM property holds an unsupported value. |
| `7` | A normalization input required by the variant is missing or invalid. |
| `8` | `--strict-dicom` refused a benchmark adaptation. |
| `9` | The input image could not be read. |
| `10` | The output image could not be written. |
