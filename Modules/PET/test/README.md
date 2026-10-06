# MitkPET tests

Most tests in this directory exercise the SUV pipeline at unit
granularity (helper, normalization strategies, input-model classifier,
filter API, per-voxel functor) and run from `mitkSUVCalculationTest.cpp`,
`mitkSUVCalculationHelperTest.cpp`, `mitkSUVFunctorPolicyTest.cpp`,
`mitkSUVImageFilterTest.cpp`, `mitkSUVInputModelTest.cpp`, and
`mitkSUVNormalizationStrategyTest.cpp`.

This document covers the two end-to-end pieces that need extra setup:
the IBSI-SUV digital reference object regression test
(`mitkPETIBSIBenchmarkTest.cpp`) and the `PETSUVCalculation` CLI smoke
tests registered from `cmake/AddPETSUVCLISmokeTests.cmake`.

## IBSI-SUV regression test

`mitkPETIBSIBenchmarkTest` drives digital reference objects (DROs) from
the [`oncoray/suv_computation`](https://github.com/oncoray/suv_computation)
benchmark through `SUVImageFilter`. The upstream tree at the pinned
commit holds **58 DROs: 43 value objects and 15 `DRO_error_*`
objects**, and the manifest carries two kinds of entry accordingly:

| `Expectation` | Asserts |
|---|---|
| `CanonicalTriple` | `(SUVmin, SUVmedian, SUVmax) = (0.20, 1.00, 4.00)` inside the shipped NIfTI ROI mask. |
| `Refusal` | `SUVImageFilter` throws; SUV is not computable from this input. |

The value phantoms are designed such that any correct SUV
implementation produces those three numbers regardless of input
pixel-unit semantics (BQML / GML / CM2ML / CNTS) or source
pre-normalization variant, so a single triple suffices for every DRO.

A `Refusal` entry additionally pins **why** the input was refused: the
expected exception type (`GetNameOfClass()`) and a substring the
message must contain. Both matter. Five of the error DROs raise
`MissingDICOMPropertyException`, so asserting the type alone would let
a regression in, say, weight handling pass behind an unrelated
missing-tag error; and the type is what the CLI's distinct exit codes
rest on, notably exit 2 (tag absent) against exit 6 (tag present,
value invalid). Keep the substrings minimal -- a tag number where there
is one -- so rewording a message does not churn the manifest.

### Where the data comes from

`cmake/PETIBSIData.cmake` resolves the DRO tree at MITK-build configure
time. Two CMake cache variables control it:

| Variable | Default | Purpose |
|---|---|---|
| `MITK_PET_DOWNLOAD_IBSI_DATA` | `ON` | Master switch. When `ON` and no checkout is supplied, the configure step `FetchContent`-clones the upstream repo at the pinned commit into `${CMAKE_BINARY_DIR}/PETIBSIData-src/`. |
| `MITK_PET_IBSI_DATA_DIR` | empty | Pre-existing checkout. When set, the configure step skips the download and validates the directory; useful for offline / proxied builds and for development against a local fork. The script never writes this variable. |

The pinned upstream commit, `MITK_PET_IBSI_DATA_GIT_TAG`, is a plain
variable in `cmake/PETIBSIData.cmake`, not a cache option: the
`BenchmarkCase` manifest in `mitkPETIBSIBenchmarkTest.cpp` is only valid
for that commit.

The data is licensed CC BY 4.0 by the IBSI / Image Biomarker
Standardisation Initiative (Vácha, Zwanenburg et al.) and is fetched
verbatim from upstream -- MITK does not redistribute it.

### Running it

After superbuild + a configure pass, the test runs as part of ctest:

```
ctest -R mitkPETIBSIBenchmark -V
```

Expected runtime: ~30 s for the current manifest (each value DRO is
~20 DICOM slices; refusal cases abort during input classification and
cost almost nothing).

### Expected status

The test runs every manifest entry unconditionally and stays
**strict-red** if any case deviates. There is no per-DRO xfail wrapper
or "expected to fail" carve-out -- every case must pass or the suite
fails. This is a deliberate, permanent invariant: the SUV pipeline must
reproduce the IBSI reference triple for every DRO it claims, and must
refuse every input it cannot compute, so a regression in any variant or
input-unit path fails the suite rather than being quietly tolerated.

The invariant is what keeps the manifest a **subset** of the upstream
tree. It contains only the cases the implementation currently
satisfies; a DRO present upstream and absent from `kBenchmarkCases` is
a declared gap, not an oversight, and the file header lists the gaps
with the reason for each. The manifest grows as each fix lands. Without
that rule, adding a DRO ahead of its fix would turn the MITK build red
for everyone.

A `Refusal` entry asserts MITK's honest-failure contract. Only the
`DRO_error_*` objects are carried that way: every value DRO is a
`CanonicalTriple`, so a green suite is the benchmark's own 58 of 58.
If a value DRO ever has to be carried as a `Refusal`, "under test" no
longer means "conformant"; keep those two figures apart in every report.

### Skip behavior

When no DRO tree is resolved (download switched off and
`MITK_PET_IBSI_DATA_DIR` empty, e.g. an offline developer build), the
test exits with code 77 at runtime and ctest reports it as Skipped --
mirroring MITK's existing missing-data convention. Primary CI must
always have the data.

### Bumping the upstream commit SHA

1. Fetch the new HEAD: `git ls-remote https://github.com/oncoray/suv_computation.git HEAD`.
2. Set `MITK_PET_IBSI_DATA_GIT_TAG` in `cmake/PETIBSIData.cmake` to the new SHA.
3. Reconfigure. Every build tree that downloads the data moves its
   checkout to the new SHA; trees with `MITK_PET_IBSI_DATA_DIR` set keep
   using that checkout and must update it themselves.
4. Walk the new `DRO/` tree and `docs/DRO_list.csv`. If the case set or
   directory layout has changed, update `kBenchmarkCases` in
   `mitkPETIBSIBenchmarkTest.cpp`. The expected `(0.20, 1.00, 4.00)`
   triple is upstream-truth; if the CSV stops carrying constants, the
   test design assumption is broken and the test must be reworked.
   Rows whose expected-value columns hold the literal `ERROR` are
   refusal cases and map to `Expectation::Refusal`. Identify them by the
   `DRO_error_` id prefix, not by the `Section` column -- upstream reuses
   `units`, `dose`, `admintime` and `halflife` for both kinds. Nothing in
   the C++ keys off `Section`.
5. Re-run `ctest -C <config> -L PET`, not just `-R mitkPETIBSIBenchmark`:
   the CLI smoke tests read the same data directory.

## CLI smoke tests

`cmake/AddPETSUVCLISmokeTests.cmake` registers ctest cases that exercise
the **business-logic layer** of `PETSUVCalculation.cpp` -- argument
parsing, exit-code dispatch (the `ExitCode` enum at the top of the
.cpp), override passthrough -- *not* the SUV computation, which is
covered by the IBSI regression test above.

Each case runs the built CLI executable through
`cmake/AssertExitCode.cmake` (a `cmake -P` wrapper) so we can pin a
*specific* non-zero exit code; CTest's `WILL_FAIL` only distinguishes
zero from non-zero, which is not enough to differentiate `exit 4`
(InvalidArguments) from `exit 5` (MultiTracerWithoutIndex) from
`exit 8` (BenchmarkAdaptationRefused).

### Running them

```
ctest -L PETSUVCLI -V
```

All cases under the `PETSUVCLI` label run unconditionally except the
end-to-end baseline case, which is registered only when
a DRO tree is resolved.
