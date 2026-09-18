# AddPETSUVCLISmokeTests.cmake
#
# Registers ctest cases that exercise the *thin business-logic layer*
# of the PETSUVCalculation CLI app -- argument parsing, exit-code
# dispatch, override passthrough -- NOT the SUV computation itself
# (covered by mitkPETIBSIBenchmarkTest).
#
# Each case wraps the executable through cmake -P AssertExitCode.cmake
# so we can pin a *specific* non-zero exit code (CTest's WILL_FAIL only
# distinguishes zero / non-zero).
#
# Cases that need IBSI DRO inputs are gated on MITK_PET_IBSI_DATA_DIR
# and skipped quietly when it is empty. Argument-parsing cases run
# unconditionally.

# The CLI executable is optional: Modules/PET/cmdapps/ creates the
# MitkPETSUVCalculation target only when BUILD_PETCmdApps or
# MITK_BUILD_ALL_APPS is set. cmdapps is processed before this directory
# (see Modules/PET/CMakeLists.txt), so when the app is built the target
# already exists here. When it is not, skip registering the smoke tests
# rather than emit a $<TARGET_FILE:MitkPETSUVCalculation> that has no
# target to resolve and fails at generate time.
if(NOT TARGET MitkPETSUVCalculation)
  return()
endif()

set(_assert_script "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AssertExitCode.cmake")
set(_cli_exe       "$<TARGET_FILE:MitkPETSUVCalculation>")

# Mirror the runtime PATH wiring the module-test macro applies so the
# freshly built EXE can find its DLLs on Windows.
mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELEASE release RELEASE)
mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEBUG   debug   DEBUG)
set(_smoke_path ${MITK_RUNTIME_PATH_RELEASE} ${MITK_RUNTIME_PATH_DEBUG} $ENV{PATH})
list(REMOVE_DUPLICATES _smoke_path)
string(REGEX REPLACE "\;" "\\\;" _smoke_path "${_smoke_path}")

# Helper: register one CLI smoke test.
#
#   _add_petsuv_cli_smoke(<test_name> <expected_exit> <args...>)
#
# Args is a CMake list (semicolon-separated) of arguments to pass to the
# CLI. Pass an empty list with quoted "" if no args.
function(_add_petsuv_cli_smoke name expected_exit)
  set(_args "${ARGN}")
  add_test(NAME ${name}
    COMMAND ${CMAKE_COMMAND}
            -DEXPECTED=${expected_exit}
            -DCMD=${_cli_exe}
            "-DARGS=${_args}"
            -P "${_assert_script}")
  set_property(TEST ${name} APPEND PROPERTY ENVIRONMENT "PATH=${_smoke_path}")
  set_property(TEST ${name} PROPERTY LABELS "PET" "PETSUVCLI")
endfunction()

# ---- Argument-parsing cases (no IBSI data needed) ---------------------
#
# MITK's CLI parser still requires --input / --output to be present
# even when --help is passed, so we do not pin a separate "--help
# exits 0" case -- that contract is owned by the parser, not by
# PETSUVCalculation, and is not part of the business logic we want
# to regression-protect here.

# Missing both --input and --output (no args at all). CLI's
# configureSettings() returns InvalidArguments(4) when required fields
# are absent.
_add_petsuv_cli_smoke(MitkPETSUVCalculationCLI_NoArgs           4 "")

# Missing --output (only --input provided, pointing at any path; we
# expect the parser to reject before reading the file).
_add_petsuv_cli_smoke(MitkPETSUVCalculationCLI_MissingOutput    4 "--input;nonexistent.dcm")

# Missing --input.
_add_petsuv_cli_smoke(MitkPETSUVCalculationCLI_MissingInput     4 "--output;out.nrrd")

# Unknown --variant value. The parser maps a closed enum set; any other
# token must be rejected as InvalidArguments.
_add_petsuv_cli_smoke(MitkPETSUVCalculationCLI_UnknownVariant   4
  "--input;in.dcm;--output;out.nrrd;--variant;not-a-real-variant")

# Unknown --patient-sex value. Same closed-enum rejection.
_add_petsuv_cli_smoke(MitkPETSUVCalculationCLI_UnknownSex       4
  "--input;in.dcm;--output;out.nrrd;--patient-sex;Q")

# ---- Cases requiring IBSI data (registered only when present) ---------
#
# These prove the CLI's strict / variant / multi-tracer wiring against
# real DICOM input.

if(MITK_PET_IBSI_DATA_DIR AND EXISTS "${MITK_PET_IBSI_DATA_DIR}/DRO/DRO_0_0/PT")
  set(_dro_pt   "${MITK_PET_IBSI_DATA_DIR}/DRO/DRO_0_0/PT")
  set(_dro_out  "${CMAKE_CURRENT_BINARY_DIR}/MitkPETSUVCalculationCLI_DRO_0_0_out.nrrd")

  # Successful end-to-end invocation on the canonical baseline DRO.
  # Validates that --input directory loading + filter wiring + --output
  # writing all line up.
  _add_petsuv_cli_smoke(MitkPETSUVCalculationCLI_BaselineRun_DRO_0_0 0
    "--input;${_dro_pt};--output;${_dro_out};--variant;bw")

  # ---- Exit-code contract ---------------------------------------------
  #
  # mitkPETIBSIBenchmarkTest drives the filter in-process and can assert
  # exception types, but the mapping from exception type to process exit
  # code exists only in the CLI and is what calling scripts branch on.
  # These cases pin that mapping; two per code is enough, because the
  # pipeline itself is covered elsewhere.
  #
  # The codes are append-only. A script that learned "2 means a tag is
  # missing" must keep being right, so a code is never reassigned even
  # when the exception hierarchy is reorganized.
  function(_add_petsuv_cli_exit_code_case name dro expected_exit)
    if(NOT EXISTS "${MITK_PET_IBSI_DATA_DIR}/DRO/${dro}/PT")
      return()
    endif()
    _add_petsuv_cli_smoke(${name} ${expected_exit}
      "--input;${MITK_PET_IBSI_DATA_DIR}/DRO/${dro}/PT;--output;${CMAKE_CURRENT_BINARY_DIR}/${name}_out.nrrd")
  endfunction()

  # 2 = MissingDICOMProperty against 6 = InvalidDICOMPropertyValue: the
  # distinction between "the tag is not there" and "the tag is there and
  # unusable", which decides whether re-exporting can help.
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit2_WeightAbsent
                                 DRO_error_2_0 2)
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit6_WeightZero
                                 DRO_error_2_1 6)

  # 3 = AmbiguousDecayTiming, here via the half-life gate on the
  # administration date. This is the one input the benchmark once
  # computed silently and wrongly, so its refusal is worth pinning at the
  # process boundary too.
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit3_UnrecoverableAdminDate
                                 DRO_error_4_1 3)

  # 11 and 12 replace the catch-all these two used to share with genuine
  # internal errors; a caller could not previously tell "this input is
  # not convertible" from "MITK broke".
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit11_UnsupportedUnits
                                 DRO_error_2_7 11)
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit12_MissingPhilipsScale
                                 DRO_error_2_6 12)

  # 13 says "this file needs per-frame support MITK does not have", which a
  # caller must be able to tell from "this file is broken". Both Enhanced PET
  # cases are pinned because they refuse through different attributes: the
  # per-frame rescale and the per-frame frame reference datetime.
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit13_PerFrameRescale
                                 DRO_7_1_0 13)
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit13_PerFrameFrameTime
                                 DRO_7_3_1 13)
endif()
