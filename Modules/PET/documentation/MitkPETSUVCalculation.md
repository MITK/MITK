
# MitkPETSUVCalculation User Guide {#MitkPETSUVCalculationPage}

[TOC]

## Overview

This command-line tool converts a PET image (in `[Bq/mL]`) into a Standardized
Uptake Value (SUV) image using the [Medical Imaging Interaction Toolkit (MITK)](https://www.mitk.org/).

**Features**

- Three normalization variants:
  - `bw`  - body-weight-normalized SUV (SUVbw). Output `[g/mL]`.
  - `lbm` - lean-body-mass-normalized SUV (SUVlbm), Janmahasatian formula. Output `[g/mL]`.
  - `bsa` - body-surface-area-normalized SUV (SUVbsa), DuBois formula. Output `[cm^2/mL]`.
- Acquisition parameters (injected dose, half-life, decay timing,
  patient body weight / height / sex) are read from the input's DICOM
  properties by default and can be overridden via dedicated CLI flags.
- DICOM `(0054,1102) Decay Correction` is consulted to determine
  whether residual decay correction is needed (`ADMIN`, `START`, `NONE`).
- Multi-item Radiopharmaceutical Information Sequences are surfaced
  honestly: the user must opt in with `--tracer-index N`.
- Modality and units checks (`PT`, `BQML`) are strict by default and
  can be bypassed explicitly.

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

| Argument      | Description                                                                                |
|---------------|--------------------------------------------------------------------------------------------|
| `--variant`   | One of `bw` (default), `lbm`, `bsa`. Selects the SUV normalization. See **Details** below. |

### Acquisition parameter overrides

These flags override the corresponding DICOM tags. If a tag is absent
and no override is given, the run aborts with a clear error.

| Argument              | DICOM tag overridden       | Required for variant         |
|-----------------------|----------------------------|------------------------------|
| `--injected-activity` | `(0018,1074)` `[Bq]`       | all                          |
| `--body-weight`       | `(0010,1030)` `[kg]`       | all                          |
| `--patient-height`    | `(0010,1020)` `[m]`        | `lbm`, `bsa`                 |
| `--patient-sex`       | `(0010,0040)` (M / F)      | `lbm`                        |
| `--half-life`         | `(0018,1075)` `[s]`        | all                          |
| `--nuclide`           | (alternative to half-life) | all (one of 18F, 68Ga, 11C, 15O) |
| `--decay-time`        | uniform decay-time `[s]`   | -  (uniform; not a substitute for DICOM strategy NONE) |

### Validation switches

| Argument                  | Description                                                                                          |
|---------------------------|------------------------------------------------------------------------------------------------------|
| `--ignore-modality-check` | Bypass the `(0008,0060)` `== "PT"` check.                                                            |
| `--ignore-units-check`    | Bypass the `(0054,1001)` `== "BQML"` check. Resulting SUV will not be physically meaningful.         |
| `--tracer-index N`        | Explicit selection for multi-item Radiopharmaceutical Information Sequence `(0054,0016)` (0-based).  |

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

### Example 2: SUVlbm using DICOM-supplied height and sex

```bash
PETSUVCalculation -i ./fdg_pet/ -o suv_lbm.nrrd --variant lbm
```

Same input as Example 1 but with the lean-body-mass variant. Patient
height `(0010,1020)` and sex `(0010,0040)` are read from DICOM in
addition to the body weight. Output is `[g/mL]` (the LBM substitutes
weight in the SUV scale numerator).

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

| Variant | Normalization formula                                          | Output unit |
|---------|----------------------------------------------------------------|-------------|
| `bw`    | scale numerator = `bodyWeight_kg * 1000` (i.e. body weight in g) | `g/mL`      |
| `lbm`   | Janmahasatian (2005): `LBM_kg = (9270 * W) / (sex-specific term * BMI + offset)`, then `* 1000` | `g/mL`      |
| `bsa`   | DuBois (1916): `BSA_m^2 = 0.007184 * W^0.425 * H_cm^0.725`, then `* 10000` (cm^2) | `cm^2/mL`   |

The math kernel is the same for all variants:

```
SUV = pixel * scaleNumerator / (injectedActivity * 2^(-decayTime / halfLife))
```

### Decay-correction strategy

The tool reads `(0054,1102) Decay Correction` and follows the standard
DICOM semantics:

- `ADMIN` - pixel data is already decay-corrected to the administration
  time. The residual decay term is `2^0 = 1` (no further correction).
- `START` - pixel data is decay-corrected to `(0008,0031) Series Time`.
  The residual decay duration is `(SeriesTime - InjectionDateTime)`,
  applied uniformly to all voxels.
- `NONE` - pixel data is not decay-corrected. The residual decay
  duration is `(AcquisitionDateTime - InjectionDateTime)` per slice.

The injection time is read from `(0018,1078) Radiopharmaceutical Start
DateTime` if available (unambiguous) or `(0018,1072) Radiopharmaceutical
Start Time` (TM-only, with a one-shot 24 h rollover correction if the
resulting decay would be negative).

The `--decay-time` flag bypasses all of the above and applies the
supplied duration uniformly to every voxel. It is intended for inputs
without DICOM timing tags (Example 5 below) or to reproduce ADMIN-style
behaviour by passing `0`. Because it is uniform-by-construction, it is
not a substitute for the per-slice handling that DICOM strategy `NONE`
performs when reading from DICOM.

### Modality and units check

By default, the tool requires `(0008,0060) Modality == "PT"` and
`(0054,1001) Units == "BQML"`. Both comparisons trim whitespace and
match case-insensitively. Variants like `Bq/ml`, `BQ/ML` are
**not** accepted; supply `--ignore-units-check` if the input uses one
of these and you accept that the SUV will not be physically meaningful.

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

## Output Format

The output is a 3D or 3D+t MITK-loadable image (recommend `.nrrd`),
pixel type `double`, geometry identical to the input. Time-resolved
DICOM properties from the input are carried over so provenance survives
in the result.

**Limitation:** The DICOM `(0054,1001) Units` tag is carried through
unchanged from the input (`BQML`) even though the SUV output unit is
no longer Bq/mL. Downstream tools that read this tag should be aware
that it does not reflect the SUV variant. Updating it to a derived-units
string is on the roadmap.
