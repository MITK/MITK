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

`mitkPETIBSIBenchmarkTest` runs every digital reference object (DRO) in
the [`oncoray/suv_computation`](https://github.com/oncoray/suv_computation)
benchmark through `SUVImageFilter` and asserts the canonical SUV triple
`(SUVmin, SUVmedian, SUVmax) = (0.20, 1.00, 4.00)` inside the shipped
NIfTI ROI mask. The phantoms are designed such that any correct SUV
implementation produces those three numbers regardless of input
pixel-unit semantics (BQML / GML / CM2ML / CNTS) or source
pre-normalization variant, so a single triple suffices for every DRO.

### Where the data comes from

`cmake/PETIBSIData.cmake` resolves the DRO tree at MITK-build configure
time. Three CMake cache variables control it:

| Variable | Default | Purpose |
|---|---|---|
| `MITK_PET_DOWNLOAD_IBSI_DATA` | `ON` | Master switch. When `ON`, the configure step `FetchContent`-clones the upstream repo into `${CMAKE_BINARY_DIR}/PETIBSIData-src/` and sets `MITK_PET_IBSI_DATA_DIR` accordingly. |
| `MITK_PET_IBSI_DATA_DIR` | empty | Pre-existing checkout. When set, the configure step skips the download and validates the directory; useful for offline / proxied builds and for development against a local fork. |
| `MITK_PET_IBSI_DATA_GIT_TAG` | pinned commit SHA | Pinned upstream commit. Bumping invalidates the in-source `BenchmarkCase` manifest in `mitkPETIBSIBenchmarkTest.cpp`; re-validate after every bump. |

The data is licensed CC BY 4.0 by the IBSI / Image Biomarker
Standardisation Initiative (Vácha, Zwanenburg et al.) and is fetched
verbatim from upstream -- MITK does not redistribute it.

### Running it

After superbuild + a configure pass, the test runs as part of ctest:

```
ctest -R mitkPETIBSIBenchmark -V
```

Expected runtime: ~30-60 s (32 DROs, each ~20 DICOM slices).

### Expected status

The test runs all 32 DROs unconditionally and stays **strict-red** if
any case deviates. There is no per-DRO xfail wrapper or "expected to
fail" carve-out -- every case must pass or the suite fails. This is a
deliberate, permanent invariant: the SUV pipeline must reproduce the
IBSI reference triple for every shipped DRO, so a regression in any
variant or input-unit path fails the suite rather than being quietly
tolerated.

### Skip behavior

When `MITK_PET_IBSI_DATA_DIR` is empty (offline / network-restricted
developer build), the test exits with code 77 at runtime and ctest
reports it as Skipped -- mirroring MITK's existing missing-data
convention. Primary CI must always have the data.

### Bumping the upstream commit SHA

1. Fetch the new HEAD: `git ls-remote https://github.com/oncoray/suv_computation.git HEAD`.
2. Set `MITK_PET_IBSI_DATA_GIT_TAG` in `cmake/PETIBSIData.cmake` to the new SHA.
3. Reconfigure to re-clone: `cmake --build ... --target ...` will trigger FetchContent.
4. Walk the new `DRO/` tree and `docs/DRO_list.csv`. If the case set or
   directory layout has changed, update `kBenchmarkCases` in
   `mitkPETIBSIBenchmarkTest.cpp`. The expected `(0.20, 1.00, 4.00)`
   triple is upstream-truth; if the CSV stops carrying constants, the
   test design assumption is broken and the test must be reworked.
5. Re-run `ctest -R mitkPETIBSIBenchmark`.

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
`MITK_PET_IBSI_DATA_DIR` resolves to a populated tree.
