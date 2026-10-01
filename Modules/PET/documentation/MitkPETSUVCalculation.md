# MitkPETSUVCalculation {#MitkPETSUVCalculationPage}

[TOC]

## Overview

MitkPETSUVCalculation converts a PET image into a Standardized Uptake Value
(SUV) image. The standard input is an activity concentration image in
`[Bq/mL]`; images that already contain SUV values (`[g/mL]`, `[cm^2/mL]` or
Philips count data with private scale factors) are recognised from their DICOM
tags and re-normalized to the requested variant instead.

Five normalization variants are available: body weight (`bw`), lean body mass
after Janmahasatian (`lbm-janma`) or James (`lbm-james128`), ideal body
weight
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

Morgan (1994) is missing from the table below on purpose. The IBSI-SUV
manual requires SUV Type `LBM` to be *convertible* and calls the formula
obsolete, so this tool reads such an image and re-normalizes from it but
will not produce a new one. It is an input variant only.

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
and height; `lbm-janma`, `lbm-james128` and `ibw` need weight,
height and sex
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
| `GML` | `LBMJANMA`, `LBMJAMES128`, `LBM`, `IBW` | Pre-normalized SUV of that variant; re-normalized to the target variant. `LBM` is the Morgan (1994) formula, `LBM_male = 1.10 * W - 0.0120 * W^2 / H^2`, `LBM_female = 1.07 * W - 0.0148 * W^2 / H^2`. |
| `GML` | `BSA` | Rejected as inconsistent (BSA SUV uses `CM2ML`). |
| `CM2ML` | absent or `BSA` | Pre-normalized SUVbsa; re-normalized to the target variant. |
| `CM2ML` | other | Rejected as inconsistent. |
| `CNTS` (Philips) with activity-scale factor | any | `pixel * factor` is `[Bq/mL]`; the kernel is applied. |
| `CNTS` (Philips) with SUV-scale factor only | absent, empty or `BW` | `pixel * factor` is SUVbw; re-normalized to the target variant. |
| `CNTS` (Philips) with SUV-scale factor only | other | Rejected (`UnsupportedPETUnitsException`). |
| `CNTS` (Philips) without factor | any | Rejected (`MissingPhilipsPETScaleException`). |
| `CNTS` (other manufacturer) | any | Rejected (`UnsupportedPETUnitsException`). |
| other or missing | any | Rejected. |

The Philips private factors `(7053,xx00)` and `(7053,xx09)` are read by the
DICOM reader and attached as the properties `mitk.pet.PhilipsSUVScale` and
`mitk.pet.PhilipsActivityScale`.

When an export carries both, the activity-concentration factor
`(7053,xx09)` wins. It yields `[Bq/mL]`, so the normalization that follows
uses the patient data this run was configured with, including any override.
The SUV-scale factor instead yields SUVbw directly, baking in whatever
weight the scanner held at acquisition time, which cannot be corrected
afterwards. A factor that is absent, empty or zero counts as unavailable.
Because the SUV-scale factor produces SUVbw by definition, it is accepted
only when `(0054,1006)` SUV Type is absent, empty or `BW`. Re-normalization of pre-normalized inputs is

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

### Administration time

The decay duration is the interval from radiopharmaceutical administration
to the reference time resolved above. How the administration instant is
established depends on which tag is present and on the radionuclide's
half-life `T`:

1. `(0018,1078)` Radiopharmaceutical Start DateTime is present and the
   resulting duration lies in `[-3600 s, 2 * T)`: it is used as stored. The
   negative floor admits dynamic scans, where acquisition legitimately
   begins shortly before administration.
2. `(0018,1078)` is present but the duration falls outside that window: the
   stored date is treated as untrustworthy. Its time of day is kept and the
   date is taken from the reference datetime instead.
3. Only `(0018,1072)` Radiopharmaceutical Start Time is present: it carries
   no date, so the date again comes from the reference datetime.

In cases 2 and 3, if the reconstructed administration instant still falls
after the reference time, it is moved back one day -- the familiar "injected
last evening, scanned this morning" case.

Both reconstructions are permitted **only when `T` is below 41400 s**. Above
that threshold an administration date that is wrong by whole days still
yields a plausible-looking SUV, so the error could not be caught downstream;
such input ends the run with exit code 3 instead. This gate is what makes a
long-lived tracer such as Zr-89 with an uptake beyond 24 h and no
`(0018,1078)` a refusal rather than a silently wrong image.

Both reconstructions are benchmark adaptations: they are applied with a
warning by default and refused with `--strict-dicom` (exit code 8).

Datetimes carrying a UTC offset are normalized by it; a datetime without one
is read as local to itself. The reference and administration instants are
always compared on the same basis.

`START` means the scanner corrected every bed and frame of the series to one
reference instant, so every slice has to be decay-corrected to that same
instant. Step 1 below decides for the whole series and applies only if every
slice carries the private datetime. Steps 2 to 4 are evaluated slice by slice
(for dynamic data, per frame too), and the first step that slice satisfies
wins. A whole-body or dynamic scan therefore typically resolves its first bed
or frame through step 2 and the others through step 3 or 4, all landing on
the same instant. Correcting a later bed to its own start instead would count
the decay between beds twice.

The `START` fallback chain is (first applicable step wins):

1. Vendor private datetime tag: Siemens `(0071,xx22)` in private block
   "SIEMENS MED PT", GE `(0009,xx0D)` in private block "GEMS_PETD_01". The
   DICOM reader attaches them as `mitk.pet.SiemensDecayDateTime` and
   `mitk.pet.GEScanDateTime`. Used as uniform reference time for all slices.
2. The slice's `(0008,0032)` Acquisition Time equals `(0008,0031)` Series
   Time (second resolution): that Acquisition Time is the reference. Applies
   to every manufacturer and needs no frame timing -- an acquisition time
   that already equals the series time identifies the reference instant on
   its own.
3. Any manufacturer except GE: per-slice reference
   `AcquisitionTime + T_ave - FrameReferenceTime`, requiring per-slice
   `(0008,0032)`, `(0054,1300)` Frame Reference Time (non-negative),
   `(0018,1242)` Actual Frame Duration (positive) and a known half-life.
   This is the general fallback, not a Siemens/Philips special case.
4. GE: per-slice reference `AcquisitionTime - FrameReferenceTime`, requiring
   per-slice `(0008,0032)` and `(0054,1300)`. It carries no `T_ave` term and
   therefore needs neither `(0018,1242)` nor a half-life.

Steps 3 and 4 are empirical formulas -- derived from observed scanner
behaviour rather than from the DICOM specification -- and count as benchmark
adaptations: they are applied with a warning by default and refused with
`--strict-dicom` (exit code 8) as soon as one slice needs them. A whole-body
or dynamic series therefore passes `--strict-dicom` only if every slice is
resolved by step 1 or 2. The warning is emitted once and states how many
slices used the formula. Applying either one to input whose
`(0008,0070)` Manufacturer is absent, empty or unrecognized emits a second
warning, because the formula cannot be verified against the scanner that
produced the data; the general rules are still applied. If no step applies
to some slice (typically no private tag and no per-slice frame timing) the
run stops with exit code 3, even if every other slice could be resolved.

`--decay-time` bypasses all of the above and applies the given duration to
every voxel. `0` reproduces `ADMIN` behaviour. Values below `-3600` s and
non-finite values are rejected (exit code 1); the floor matches the one the
DICOM-derived path uses, so a dynamic scan the pipeline computes by itself
can also be supplied by hand. Because the value is uniform, it is not a
substitute for the per-slice handling of `START` and `NONE`.

### Supported SOP classes

| SOP class | Support |
|-----------|---------|
| PET Image Storage `1.2.840.10008.5.1.4.1.1.128` | Full. |
| Enhanced PET Image Storage `1.2.840.10008.5.1.4.1.1.130` | Supported, including a rescale and frame timing that vary per frame. MITK does not apply the Real World Value Mapping; see below. |
| Legacy Converted Enhanced PET `1.2.840.10008.5.1.4.1.1.128.1` | Not supported. |

An Enhanced PET object carries none of the classic PET attributes. The unit
comes from the Measurement Units Code Sequence `(0040,08EA)` inside the Real
World Value Mapping Sequence `(0040,9096)` inside the functional groups,
falling back to `(0028,1054)` Rescale Type; the decay state comes from
`(0018,9758)` Decay Corrected instead of `(0054,1102)`, with the reference
instant taken from `(0018,9701)` when it is `YES` and, per frame, from
`(0018,9151)` Frame Reference DateTime (or `(0018,9074)` Frame Acquisition
DateTime plus the average count-rate time) when it is `NO`. Administration
time is resolved exactly as for classic PET.

The DICOM reader applies each frame's Pixel Value Transformation when it
loads the object and publishes every functional-group attribute with one
value per slice, so a rescale slope, intercept or frame time that differs
between frames is an ordinary input. When `(0018,9758)` is `YES` the
reference instant is the single top-level `(0018,9701)` and the per-frame
frame times play no part in the result.

The Real World Value Mapping is applied by nobody. The loaded values are the
stored values through the Pixel Value Transformation, so MITK names their
unit by the mapping whose slope and intercept equal that transformation --
which is the case whenever the object was written with the two in
agreement, as every reference object is. Where several mappings qualify,
the one yielding SUVbw is preferred, then any other SUV type, then activity
concentration.

#### What is not supported, and why it refuses

- A unit that differs between frames. MITK carries one unit per image, so
  the input cannot be represented; the run stops with exit code 13.
- A multi-frame object whose Per-Frame Functional Groups Sequence does not
  carry one item per frame. The reader cannot map its functional groups to
  frames and publishes none of their values; the run stops with exit
  code 15 rather than reporting the unit as absent.
- An object none of whose mappings equals the applied Pixel Value
  Transformation, a mapping through a Real World Value LUT included. The
  loaded values are then in no unit the object declares, and the run stops
  with exit code 14 rather than scaling by a guess. This is the
  standard-conformant layout -- Rescale Type `US`, an arbitrary
  transformation, the real mapping in the Real World Value Mapping -- so it
  is a MITK limitation, not a defect in the input, and the message says so.

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
| Empirical decay-timing fallback for `START` | steps 3 or 4 of the fallback chain | applied | `VendorEmpiricalDecayFallbackRefusedException` |
| Unverified manufacturer for that fallback | `(0008,0070)` absent, empty or unrecognized *and* step 3 or 4 fires | general rule applied | covered by the row above |
| Patient sex `O` for a sex-specific variant | `(0010,0040)` or `--patient-sex` is `O` | mean of male and female numerators | `AmbiguousPatientSexAdaptationRefusedException` |
| Administration date rebuilt from the reference datetime | duration outside `[-3600 s, 2 * T)`, or only `(0018,1072)` present | stored time of day kept, date taken from the reference | `AdministrationDateSubstitutionRefusedException` |
| Patient's Weight `(0010,1030)` given in grams | value `>= 1000` | divided by 1000 | `ImplausiblePatientWeightException` |

Every adaptation that fires is also recorded, so it can be audited without
parsing the log. The record holds one entry per applied recommendation,
naming the rule, the DICOM tag concerned, the stored value and the value
used, and it leaves the process three ways:

- **On the console.** After a successful run the CLI prints a consolidated
  block listing every adaptation, regardless of `--verbose`. The per-rule
  warnings the pipeline emits while it works are interleaved with everything
  else; this is the one place the whole set appears together.
- **In the output image**, as the properties described under
  [Output](#MitkPETSUVCalculationOutput), which survive a save.
- **In process**, via `mitk::SUVImageFilter::GetAdaptations()`, for a caller
  embedding the filter.

Under `--strict-dicom` that record is necessarily empty, because the first
adaptation raises instead of being applied -- an empty record under the
strict policy is the guarantee the mode exists to give, not a lack of
information.

DICOM prescribes kilograms for `(0010,1030)`, and no patient weighs 1000 of
them, so a value at or above that identifies a gram-encoded export. Read at
face value it makes the weight 1000x too large and every SUV 1000x too small.
`--body-weight` bypasses the reinterpretation entirely.

### Rescale slope and intercept

`(0028,1053)` Rescale Slope and `(0028,1052)` Rescale Intercept are read for
validation only and never re-applied: the reader has already scaled the pixel
buffer with them by the time the SUV pipeline sees the image, so applying them
again would scale twice. A warning is emitted when the slope is absent or
non-positive, or the intercept absent or non-zero -- an absent slope in
particular is silently treated as `1.0`, which is worth knowing about. These
are diagnostics, not adaptations: nothing is reinterpreted, so nothing is
recorded and `--strict-dicom` does not refuse them. Enhanced PET objects carry
no top-level rescale and are validated by their own classifier instead.

They are also collected and reported together after a successful run, next to
the adaptation summary, so they do not have to be found among the rest of the
log. An input carrying no rescale tags at all -- a plain NRRD driven entirely
by overrides, for instance -- reports both as absent. That is intended: an
image whose activity scale cannot be confirmed is exactly the case the
recommendation is about, whatever the reason.

`--injected-activity` bypasses the dose adaptation and `--decay-time` bypasses
the decay-timing fallback regardless of `--strict-dicom`; both values are used
verbatim.

### Output {#MitkPETSUVCalculationOutput}

The output image has pixel type `double`, the geometry and time steps of the
input, and a copy of the input's properties (including the DICOM properties).
`(0054,1001)` Units and `(0054,1006)` SUV Type are set according to the target
variant; `(0028,1052)` Rescale Intercept is set to `0.0` and `(0028,1053)`
Rescale Slope to `1.0`:

| Variant | `(0054,1001)` | `(0054,1006)` |
|---------|---------------|---------------|
| `bw` | `GML` | `BW` |
| `lbm-janma` | `GML` | `LBMJANMA` |
| `lbm-james128` | `GML` | `LBMJAMES128` |
| `ibw` | `GML` | `IBW` |
| `bsa` | `CM2ML` | `BSA` |

The adaptation record is written to the output as well, so an SUV image
carries its own provenance:

| Property | Content |
|----------|---------|
| `mitk.pet.suv.adaptations` | The record as a JSON array; `[]` when nothing was adapted. Each entry has `rule`, `dicomTag`, `originalValue` and `usedValue`. |
| `(0008,2111)` Derivation Description | `MITK SUV`, followed by the number of adaptations when there were any. The tag is `LO` and holds the count, not the record. |

Where an adaptation reinterpreted a tag, the output carries the value the
computation actually used rather than the input's original: the corrected
`(0010,1030)` Patient's Weight in kilograms, and the corrected
`(0054,0016)[0].(0018,1074)` Radionuclide Total Dose in becquerel. Without
this the output would inherit the misleading original, and a reader taking
`(0010,1030)` at face value would conclude the SUV came from a one-tonne
patient.

The reconstructed administration datetime is deliberately *not* mirrored onto
`(0018,1078)`. The record holds what the substitution consumed -- the
implausible duration, or the tag that drove it -- but never the resolved
datetime itself, because the computation needs only the decay duration and
never forms one. Writing the tag would mean deriving a value that does not
otherwise exist, per slice for a `START` correction, purely to restate what
the record already says. The record names the substitution, the tag that
triggered it and the original value, which is what a reader needs.

DICOM defines a distinct SUV Type for each lean-body-mass formula, and the
written tag names the one actually used. An SUV image is therefore readable
as an input again: feeding it back re-normalizes from the correct source
variant. Earlier versions collapsed all lean-body-mass variants onto the
generic `LBM`, which made the output unreadable by this tool; if you hold
such a file, its SUV Type does not identify the formula and the value has to
come from elsewhere.

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
or `--variant lbm-james128` for the James formula.

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
| `11` | `(0054,1001)` Units holds a value the pipeline cannot convert. |
| `12` | `Units = CNTS` on Philips data without either private scale factor. |
| `13` | The frames of an Enhanced PET object name different units; MITK carries one unit per image. |
| `14` | No Real World Value Mapping of an Enhanced PET object describes the loaded pixel values; MITK does not apply the mapping. |
| `15` | The DICOM reader could not map the functional groups of a multi-frame Enhanced PET object to frames, so none of its per-frame values reached MITK. |

Codes 11 and 12 previously fell into the catch-all `1`, so a calling script
could not tell "this input is not convertible" from "MITK broke". The table
is append-only: a code, once published, keeps its meaning.
