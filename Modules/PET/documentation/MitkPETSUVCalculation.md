
# MitkPETSUVCalculation User Guide {#MitkPETSUVCalculationPage}

[TOC]

## Overview

This command-line tool converts a PET image (in `[Bq/mL]`) into a Standardized
Uptake Value (SUV) image using the [Medical Imaging Interaction Toolkit (MITK)](https://www.mitk.org/).

**Features**

- Five normalization variants:
  - `bw`            - body-weight-normalized SUV (SUVbw). Output `[g/mL]`.
  - `lbm-janma`     - lean-body-mass-normalized SUV (SUVlbm), Janmahasatian (2005)
    formula. IBSI-SUV-recommended LBM formula. Output `[g/mL]`.
  - `lbm-james128`  - lean-body-mass-normalized SUV using the James (1976)
    "James 128" formula. Output `[g/mL]`.
  - `ibw`           - ideal-body-weight-normalized SUV using the Sugawara
    (1999) sex-specific formula (also IBSI-SUV-recommended). Output `[g/mL]`.
  - `bsa`           - body-surface-area-normalized SUV (SUVbsa), DuBois (1916)
    formula. Output `[cm^2/mL]`.
- Five input pixel-unit semantics, classified from `(0054,1001) Units`:
  - `BQML`  - activity concentration `[Bq/mL]` (standard).
  - `GML`   - pre-computed SUVbw `[g/mL]` (re-scaled to the target variant).
  - `CM2ML` - pre-computed SUVbsa `[cm^2/mL]` (re-scaled to the target variant).
  - `CNTS` + Philips SUV-scale private factor   - pixel * factor yields SUV directly.
  - `CNTS` + Philips activity-scale private factor - pixel * factor yields `[Bq/mL]`.
- Acquisition parameters (injected dose, half-life, decay timing,
  patient body weight / height / sex) are read from the input's DICOM
  properties by default and can be overridden via dedicated CLI flags.
- DICOM `(0054,1102) Decay Correction` is consulted to determine
  whether residual decay correction is needed (`ADMIN`, `START`, `NONE`).
- Multi-item Radiopharmaceutical Information Sequences are surfaced
  honestly: the user must opt in with `--tracer-index N`.
- Modality check (`PT`) is strict by default and can be bypassed
  explicitly. The units check is integrated into the input-units
  classifier; legacy `--ignore-units-check` forces activity-concentration
  semantics regardless of the tag.

## Basic Usage

```bash
PETSUVCalculation -i <input> -o <output.nrrd> [options]
```

If the input is a DICOM PET dataset with the standard tags, the default
invocation needs nothing beyond `-i` and `-o`.

## Required Arguments

| Argument          | Description                                                       |
|-------------------|-------------------------------------------------------------------|
| `-i`, `--input`   | Input PET image (DICOM dir or any MITK-supported format).         |
| `-o`, `--output`  | Output file path for the resulting SUV image (recommend `.nrrd`). |

## Optional Arguments

### Variant selection

| Argument      | Description                                                                                                                            |
|---------------|----------------------------------------------------------------------------------------------------------------------------------------|
| `--variant`   | One of `bw` (default), `lbm-janma`, `lbm-james128`, `ibw`, `bsa`. Selects the **output** SUV normalization. See **Details** below.    |

`--variant` controls the variant of the SUV image the tool produces. For
activity-concentration inputs (`Units=BQML`, the standard case) the tool
applies the variant's formula directly. For pre-normalized inputs
(`Units=GML`, `CM2ML`, or Philips `CNTS+SUV-scale`) the tool first
classifies the input variant from `(0054,1001) Units` and `(0054,1006)
SUV Type`, then re-normalizes from the source variant to the target
`--variant`. Per the IBSI-SUV spec the canonical, vendor-neutral output
of any conversion is `SUVbw`, so the default `bw` is the correct choice
for processing pre-normalized inputs unless you specifically need the
output in a different variant.

### Acquisition parameter overrides

These flags override the corresponding DICOM tags. If a tag is absent
and no override is given, the run aborts with a clear error.

| Argument              | DICOM tag overridden       | Required for variant                                            |
|-----------------------|----------------------------|-----------------------------------------------------------------|
| `--injected-activity` | `(0018,1074)` `[Bq]`       | activity-concentration inputs only                              |
| `--body-weight`       | `(0010,1030)` `[kg]`       | always                                                          |
| `--patient-height`    | `(0010,1020)` `[m]`        | `lbm-janma`, `lbm-james128`, `ibw`, `bsa`                       |
| `--patient-sex`       | `(0010,0040)` (M / F / O)  | `lbm-janma`, `lbm-james128`, `ibw`                              |
| `--half-life`         | `(0018,1075)` `[s]`        | activity-concentration inputs only                              |
| `--nuclide`           | (alternative to half-life) | activity-concentration inputs only (one of 18F, 68Ga, 11C, 15O) |
| `--decay-time`        | uniform decay-time `[s]`   | -  (uniform; not a substitute for DICOM strategy NONE)          |

Pre-normalized SUV inputs (`Units = GML`, `CM2ML`, or Philips
`CNTS + SUV-scale`) bypass the activity-to-SUV math entirely and only
re-normalize from the source variant to the target variant; injected
dose, half-life, and decay timing are not consulted for those inputs.

### Validation switches

| Argument                  | Description                                                                                          |
|---------------------------|------------------------------------------------------------------------------------------------------|
| `--ignore-modality-check` | Bypass the `(0008,0060)` `== "PT"` check.                                                                                                                                                                                                       |
| `--ignore-units-check`    | Force activity-concentration semantics regardless of `(0054,1001)`. Equivalent to passing the input through the standard SUV formula as if `Units = BQML`. Bypasses both the input-units classifier and any pre-normalized re-scale path.   |
| `--strict-dicom`          | Refuse benchmark-recommended adaptations of borderline DICOM input (see *Details*). Default: off.                                                                                                                                               |
| `--tracer-index N`        | Explicit selection for multi-item Radiopharmaceutical Information Sequence `(0054,0016)` (0-based).                                                                                                                                             |

### Misc

| Argument         | Description           |
|------------------|-----------------------|
| `-v`, `--verbose`| Verbose output.       |
| `-h`, `--help`   | Show help text.       |

## Examples

### Example 1: SUVbw with full DICOM auto-detection

```bash
PETSUVCalculation -i ./fdg_pet/ -o suv_bw.nrrd
```

A DICOM PET series with all the standard tags. Half-life, injected dose,
decay timing, and patient weight are all read from DICOM. Output is
`[g/mL]`.

---

### Example 2: SUVlbm (Janmahasatian) using DICOM-supplied height and sex

```bash
PETSUVCalculation -i ./fdg_pet/ -o suv_lbm.nrrd --variant lbm-janma
```

Same input as Example 1 but with the lean-body-mass variant
(Janmahasatian, IBSI-SUV-recommended LBM formula). Patient height
`(0010,1020)` and sex `(0010,0040)` are read from DICOM in addition to
the body weight. Output is `[g/mL]` (the LBM substitutes weight in the
SUV scale numerator).

The alternative `lbm-james128` variant uses the older James (1976)
"James 128" formula instead.

---

### Example 3: SUVbsa with manual `--patient-height`

```bash
PETSUVCalculation \
  -i ./fdg_pet/ -o suv_bsa.nrrd \
  --variant bsa --patient-height 1.78
```

Body surface area normalization (DuBois formula). The patient-height
DICOM tag is missing in this dataset, so it is supplied via the override.
Output is `[cm^2/mL]`.

---

### Example 4: Multi-tracer dataset

```bash
PETSUVCalculation \
  -i ./multi_tracer_pet/ -o suv.nrrd \
  --tracer-index 1
```

The Radiopharmaceutical Information Sequence has more than one item. The
tool refuses to silently pick one; the user names the desired item
explicitly.

---

### Example 5: NRRD input without DICOM tags

```bash
PETSUVCalculation \
  -i pet.nrrd -o suv.nrrd \
  --injected-activity 1.85e8 \
  --nuclide 18F \
  --body-weight 70 \
  --decay-time 0 \
  --ignore-modality-check --ignore-units-check
```

A non-DICOM input; all parameters supplied via CLI. `--decay-time 0`
matches the DICOM `ADMIN` semantics (pixel data already decay-corrected
to administration time). Modality and units checks are bypassed since
NRRD does not carry the corresponding DICOM tags.

## Details

### Normalization variants

| Variant         | Normalization formula                                                                                                                                                                                                            | Output unit |
|-----------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------|
| `bw`            | Body weight: scale numerator = `W_kg * 1000`. Reference: Strauss & Conti, *J Nucl Med* 1991.                                                                                                                                     | `g/mL`      |
| `lbm-janma`     | Janmahasatian (2005): `LBM_male = (9270*W)/(6680+216*BMI)`, `LBM_female = (9270*W)/(8780+244*BMI)`, then `*1000`. **IBSI-SUV-recommended LBM formula.**                                                                          | `g/mL`      |
| `lbm-james128`  | James (1976): `LBM_male = 1.10*W - 0.0128*W^2/H^2`, `LBM_female = 1.07*W - 0.0148*W^2/H^2` (W in kg, H in m), then `*1000`.                                                                                                      | `g/mL`      |
| `ibw`           | Sugawara (1999): `IBW_male = 48.0 + 1.06*(H_cm - 152)`, `IBW_female = 45.5 + 0.91*(H_cm - 152)`, then `*1000`. Below 152 cm the formula returns values below the constant; the tool does not clamp. Also adopted verbatim by the IBSI-SUV benchmark.   | `g/mL`      |
| `bsa`           | DuBois (1916): `BSA_m² = 0.007184 * W^0.425 * H_cm^0.725`, then `*10000` (cm²).                                                                                                                                                  | `cm²/mL`    |

The math kernel is the same for all variants on activity-concentration
inputs:

```
SUV = pixel * scaleNumerator / (injectedActivity * 2^(-decayTime / halfLife))
```

Pre-normalized inputs (Units = `GML`, `CM2ML`, or Philips
`CNTS+SUV-scale`) skip the kernel and re-normalize from the source
variant to the target variant:

```
SUV_target = pixel * prenormScale * scaleNumerator(target) / scaleNumerator(source)
```

#### Patient sex `O` (Other) under `--strict-dicom`

For the sex-specific variants (`lbm-janma`, `lbm-james128`, `ibw`),
patient sex `O` (Other) is handled by the IBSI-SUV-recommended
adaptation: the SUV is computed as the mean of the male- and
female-specific scale numerators. By default this adaptation is applied
silently with a `MITK_WARN`. Under `--strict-dicom` it is refused and
the tool exits with code `8` and an
`AmbiguousPatientSexAdaptationRefusedException` message; supply
`--patient-sex M|F` to disambiguate or relax the policy.

### Input pixel-unit semantics

The tool classifies the input's pixel semantics from `(0054,1001) Units`
*and* `(0054,1006) SUV Type` per the IBSI-SUV spec, then dispatches to
the appropriate pipeline. `(0054,1001)` carries the pixel unit
(`g/mL`, `cm^2/mL`, `[Bq/mL]`, `count`); `(0054,1006)` carries the actual
SUV variant the pixel represents.

| `(0054,1001)`                    | `(0054,1006)`                                       | Pipeline                                                | Notes                                                                                       |
|----------------------------------|-----------------------------------------------------|---------------------------------------------------------|---------------------------------------------------------------------------------------------|
| `BQML`                           | (ignored)                                           | activity-to-SUV                                         | Standard case; pixel is `[Bq/mL]`.                                                          |
| `GML`                            | empty / absent / `BW`                               | renormalize from SUVbw                                  | Pixel is SUVbw `[g/mL]`; re-scale to target variant.                                        |
| `GML`                            | `LBMJANMA`                                          | renormalize from SUVlbm-janma                           | Pixel is SUVlbm (Janmahasatian).                                                            |
| `GML`                            | `LBMJAMES128`                                       | renormalize from SUVlbm-james128                        | Pixel is SUVlbm (James 128).                                                                |
| `GML`                            | `IBW`                                               | renormalize from SUVibw                                 | Pixel is SUVibw.                                                                            |
| `GML`                            | `LBM` (Morgan)                                      | refuse                                                  | IBSI-SUV calls this obsolete and not covered. Re-export the input with a specific LBM formula tag. |
| `GML`                            | `BSA`                                               | refuse (inconsistent)                                   | BSA SUV uses `Units = CM2ML`, not GML.                                                      |
| `CM2ML`                          | absent / `BSA`                                      | renormalize from SUVbsa                                 | Pixel is SUVbsa `[cm^2/mL]`.                                                                |
| `CM2ML`                          | other                                               | refuse (inconsistent)                                   | CM2ML is strictly coupled to `BSA`.                                                         |
| `CNTS` + Philips SUV-scale       | (consulted; default `BW`)                           | renormalize from SUVbw                                  | Manufacturer must be Philips; pixel * `(7053,xx00)` factor yields SUVbw, then re-scale.     |
| `CNTS` + Philips activity-scale  | (ignored)                                           | activity-to-SUV                                         | Manufacturer must be Philips; pixel * `(7053,xx09)` factor yields `[Bq/mL]`, standard math. |
| `CNTS` without Philips factor    | (any)                                               | refuse                                                  | Both Lenient and Strict raise `MissingPhilipsPETScaleException`.                            |
| `CNTS` non-Philips manufacturer  | (any)                                               | refuse                                                  | Both Lenient and Strict raise `UnsupportedPETUnitsException`.                               |
| Anything else / missing          | (any)                                               | refuse                                                  | The IBSI-SUV catalogue defines no fallback; both policies raise.                            |

The Philips private factors are lifted automatically by the PET DICOM
reader and attached as named properties (`mitk.pet.PhilipsSUVScale`,
`mitk.pet.PhilipsActivityScale`).

`--ignore-units-check` is the legacy escape hatch: it forces
activity-concentration semantics regardless of the actual `(0054,1001)`
value. Use it only if you know the pixels really are `[Bq/mL]`.

### Output DICOM tags

The tool sets `(0054,1001) Units` and `(0054,1006) SUV Type` on the
output to match the chosen target variant:

| Target variant   | Output `(0054,1001)` | Output `(0054,1006)` |
|------------------|----------------------|----------------------|
| `bw`             | `GML`                | `BW`                 |
| `lbm-janma`      | `GML`                | `LBM`                |
| `lbm-james128`   | `GML`                | `LBM`                |
| `ibw`            | `GML`                | `IBW`                |
| `bsa`            | `CM2ML`              | `BSA`                |

Both DICOM-defined LBM variants share the standard SUV Type code `LBM`;
the chosen formula is recorded externally (e.g. via filename or in a
study log). `(0028,1052) RescaleIntercept` is set to `0.0` and
`(0028,1053) RescaleSlope` to `1.0` since SUV is already in physical
units.

### Decay-correction strategy

The tool reads `(0054,1102) Decay Correction` and follows the IBSI-SUV-
benchmark-recommended interpretation:

- `ADMIN` - pixel data is already decay-corrected to the administration
  time. The residual decay term is `2^0 = 1` (no further correction).
- `START` - pixel data is decay-corrected to a vendor-specific reference
  time. See *DC=START fallback chain* below.
- `NONE` - pixel data is not decay-corrected. The voxel value
  represents the count rate averaged over the frame, which equals the
  instantaneous rate at `AcquisitionDateTime + T_ave`, with `T_ave` the
  closed-form average count-rate time
  `T_ave = (1/lambda) * ln((lambda*T) / (1 - exp(-lambda*T)))`,
  `lambda = ln(2)/T_half`, `T = (0018,0x1242) ActualFrameDuration`.
  The residual decay duration is therefore
  `(AcquisitionDateTime + T_ave) - InjectionDateTime` per slice.
  Inputs that lack `(0018,0x1242)` are rejected with a
  `MissingDICOMPropertyException` rather than fall back to the biased
  `(t_acq - t_inj)` form (the bias is ~3 % for typical 5-minute
  frames); use `--decay-time` to supply timing manually.

The injection time is read from `(0018,1078) Radiopharmaceutical Start
DateTime` if available (unambiguous) or `(0018,1072) Radiopharmaceutical
Start Time` (TM-only, with a one-shot 24 h rollover correction if the
resulting decay would be negative).

#### DC=START fallback chain

The reference time used to compute the residual decay correction is
chosen by the IBSI-SUV-benchmark-recommended fallback chain — *the first
condition whose preconditions are met determines the result*:

1. **Vendor private datetime tag.**
   - Siemens: `(0071,0x22)` in private block `"SIEMENS MEDCOM HEADER"`.
   - GE: `(0009,0x0D)` in private block `"GEMS_PETD_01"`.
   The PET reader lifts these out of the dataset at load time and
   attaches them as named properties (`mitk.pet.SiemensDecayDateTime` /
   `mitk.pet.GEScanDateTime`); see issue #783 for the design status.
   When present and yielding a non-negative decay, the value is used as
   the uniform reference time across all slices.

2. **`(0008,0032) AcquisitionTime` equals `(0008,0031) SeriesTime`** in
   seconds at slice 0, and Manufacturer is one of Siemens, GE, Philips.
   Per-slice AcquisitionTime is then used as the reference, accommodating
   sub-second per-slice variation while honouring the SeriesTime tie.

3. **Siemens / Philips, vendor T_ave formula.** Per-slice reference =
   `AcquisitionTime + T_ave - FrameReferenceTime`, with `T_ave` from the
   closed-form above. The `-FrameReferenceTime` term undoes the
   scanner-applied offset from the per-frame midpoint back to the start
   of acquisition; `T_ave` additionally compensates for the average
   count-rate time inside the frame. Requires per-slice `(0008,0032)`
   AcquisitionTime, `(0054,0x1300)` FrameReferenceTime (non-negative),
   `(0018,0x1242)` ActualFrameDuration (positive), and a known half-life.

4. **GE, vendor `Δt` formula.** Per-slice reference =
   `AcquisitionTime - FrameReferenceTime`. Requires the same per-slice
   tags as Step 3 (with the same non-negativity / positivity preconditions).

If none of these applies — typically an unrecognized manufacturer with
no private tag and no per-slice frame timing — the helper raises an
`AmbiguousDecayTimingException`. The tool refuses to silently extend a
vendor-specific formula to an input it cannot classify, and exits with
code `3`. Use `--decay-time` to supply timing externally.

#### `--decay-time` escape hatch

The `--decay-time` flag bypasses all of the above and applies the
supplied duration uniformly to every voxel. It is intended for inputs
without DICOM timing tags (Example 5 above) or to reproduce ADMIN-style
behaviour by passing `0`. Because it is uniform-by-construction, it is
not a substitute for the per-slice handling that DC=START / DC=NONE
perform when reading from DICOM.

### Modality check

By default, the tool requires `(0008,0060) Modality == "PT"` (trim and
case-insensitive). Bypass with `--ignore-modality-check` if the input
is a valid PET image whose modality tag is missing or non-standard.

The units check is no longer a binary gate; `(0054,1001) Units` is
classified into one of the supported pixel-semantics categories
documented in *Input pixel-unit semantics* above. Use
`--ignore-units-check` only when you want to force
activity-concentration semantics regardless of the tag (the legacy
behaviour: pixels are assumed `[Bq/mL]` and the standard SUV formula
applies).

### IBSI-SUV benchmark adaptations and `--strict-dicom`

The IBSI-SUV benchmark catalogues a small number of recommendations
that help MITK accept real-world PET DICOM input without losing
physical meaning. Each one has a clearly bounded trigger, and by
default the tool applies the recommendation and emits a `MITK_WARN`
log entry so post-processing pipelines can audit the adaptation.

Pass `--strict-dicom` to refuse all such adaptations; the tool then
exits with code `8` (`BenchmarkAdaptationRefused`) and an error
message identifying the offending input. Strict mode is intended for
validation, regulatory, or strict-conformance contexts where silent
reinterpretation of borderline input is unacceptable.

The currently implemented adaptations:

| Adaptation                                       | Trigger                                              | Lenient response                                            | Strict response                                                                       |
|--------------------------------------------------|------------------------------------------------------|-------------------------------------------------------------|---------------------------------------------------------------------------------------|
| Radionuclide Total Dose `(0018,1074)` MBq detect | DICOM-side value strictly between `0` and `1e4`      | Multiply by `1e6` (interpret as MBq), emit `MITK_WARN`.     | Exit `8` with an `ImplausibleRadionuclideDoseException` message.                      |
| DC=START vendor empirical decay-timing fallback  | DC=START, no Step 1 / Step 2 match, but Step 3 / 4 preconditions met (Siemens / Philips T_ave / GE -ΔFrameRef) | Apply the IBSI-SUV recommended formula. | Exit `8` with a `VendorEmpiricalDecayFallbackRefusedException`; supply unambiguous timing or `--decay-time`. |
| Sex `(0010,0040) == 'O'` mean-of-M-and-F         | sex-specific target variant (`lbm-janma`, `lbm-james128`, `ibw`) and patient sex resolves to `Other` | Compute the mean of the male- and female-specific scale numerators, emit `MITK_WARN`. | Exit `8` with an `AmbiguousPatientSexAdaptationRefusedException`; supply `--patient-sex M\|F`. |

The threshold for the dose adaptation sits in the empirically empty
gap between the two physical regimes (clinical FDG doses cluster
around `4e2` MBq and `4e8` Bq), so it cannot misclassify a plausible
clinical input. The decay-timing fallback is empirical-not-derivable:
the formulas were observed across real Siemens / Philips / GE PET
data, so Strict refuses them and forces the caller to supply a
spec-clean reference (vendor private datetime, AcquisitionTime equal
to SeriesTime) or an explicit `--decay-time`.

The `--injected-activity` override bypasses the dose adaptation
entirely (regardless of `--strict-dicom`); the `--decay-time`
override likewise bypasses the decay-timing fallback entirely. Both
accept the value verbatim and are treated as opt-outs from the
respective heuristic.

### Exit codes

| Code | Meaning                                                          |
|------|------------------------------------------------------------------|
| `0`  | Success                                                          |
| `1`  | Generic / unexpected error                                       |
| `2`  | A required DICOM property is missing                             |
| `3`  | Ambiguous decay timing (cannot reconcile injection / acquisition) |
| `4`  | Invalid CLI arguments                                            |
| `5`  | Multi-item Radiopharmaceutical Sequence without `--tracer-index` |
| `6`  | A DICOM property holds an unsupported value                      |
| `7`  | A required SUV input (e.g. height for `lbm`) is missing          |
| `8`  | `--strict-dicom`: a benchmark-recommended adaptation was refused |

## Output Format

The output is a 3D or 3D+t MITK-loadable image (recommend `.nrrd`),
pixel type `double`, geometry identical to the input. Time-resolved
DICOM properties from the input are carried over so provenance survives
in the result.

The output `(0054,1001)` and `(0054,1006)` DICOM tags are set per the
target variant (see *Output DICOM tags* above), so downstream tools see
a consistent description of the SUV result.
