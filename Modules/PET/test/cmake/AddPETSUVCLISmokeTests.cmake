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
# Cases that need IBSI DRO inputs are gated on MITK_PET_IBSI_RESOLVED_DATA_DIR
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
#
# Two optional keyword arguments, each taking one regex:
#
#   EXPECT_OUTPUT <regex>   the run must print something matching it
#   REJECT_OUTPUT <regex>   the run must print nothing matching it
#
# They pin contracts an exit code cannot carry -- a successful run that
# still owes the operator a diagnostic, or one that must stay quiet.
function(_add_petsuv_cli_smoke name expected_exit)
  cmake_parse_arguments(_smoke "" "EXPECT_OUTPUT;REJECT_OUTPUT" "" ${ARGN})
  set(_args "${_smoke_UNPARSED_ARGUMENTS}")

  set(_output_assertions "")
  if(DEFINED _smoke_EXPECT_OUTPUT)
    list(APPEND _output_assertions "-DEXPECT_OUTPUT=${_smoke_EXPECT_OUTPUT}")
  endif()
  if(DEFINED _smoke_REJECT_OUTPUT)
    list(APPEND _output_assertions "-DREJECT_OUTPUT=${_smoke_REJECT_OUTPUT}")
  endif()

  add_test(NAME ${name}
    COMMAND ${CMAKE_COMMAND}
            -DEXPECTED=${expected_exit}
            -DCMD=${_cli_exe}
            "-DARGS=${_args}"
            ${_output_assertions}
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

if(MITK_PET_IBSI_RESOLVED_DATA_DIR AND EXISTS "${MITK_PET_IBSI_RESOLVED_DATA_DIR}/DRO/DRO_0_0/PT")
  set(_dro_pt   "${MITK_PET_IBSI_RESOLVED_DATA_DIR}/DRO/DRO_0_0/PT")
  set(_dro_out  "${CMAKE_CURRENT_BINARY_DIR}/MitkPETSUVCalculationCLI_DRO_0_0_out.nrrd")

  # Successful end-to-end invocation on the canonical baseline DRO.
  # Validates that --input directory loading + filter wiring + --output
  # writing all line up.
  _add_petsuv_cli_smoke(MitkPETSUVCalculationCLI_BaselineRun_DRO_0_0 0
    "--input;${_dro_pt};--output;${_dro_out};--variant;bw"
    REJECT_OUTPUT "IBSI-SUV input adaptations applied")

  # DRO_3_2_3 needs two recommendations to be computable at all: the
  # empirical DC=START formula, and applying it to a manufacturer the
  # formula was never validated against. A run that reinterprets its input
  # that far and says nothing is the failure this case exists to prevent,
  # and no exit code can express it -- the run legitimately succeeds.
  # The baseline case above holds the other half: silence when the input
  # needed nothing, so that "always warns" cannot pass both.
  if(EXISTS "${MITK_PET_IBSI_RESOLVED_DATA_DIR}/DRO/DRO_3_2_3/PT")
    _add_petsuv_cli_smoke(MitkPETSUVCalculationCLI_ReportsAdaptations_DRO_3_2_3 0
      "--input;${MITK_PET_IBSI_RESOLVED_DATA_DIR}/DRO/DRO_3_2_3/PT;--output;${CMAKE_CURRENT_BINARY_DIR}/MitkPETSUVCalculationCLI_DRO_3_2_3_out.nrrd;--variant;bw"
      EXPECT_OUTPUT "VendorEmpiricalDecayFallback")
  endif()

  # ---- Exit-code contract ---------------------------------------------
  #
  # mitkPETIBSIBenchmarkTest drives the filter in-process and can assert
  # exception types, but the mapping from exception type to process exit
  # code exists only in the CLI and is what calling scripts branch on.
  # The cases below pin that mapping for every code a DRO provokes, one
  # input per code, because the pipeline itself is covered elsewhere.
  # Codes 13, 14 and 15 have no DRO: the unit tests pin their exception
  # types, and their exit-code mapping is not verified here.
  #
  # The codes are append-only. A script that learned "2 means a tag is
  # missing" must keep being right, so a code is never reassigned even
  # when the exception hierarchy is reorganized.
  function(_add_petsuv_cli_exit_code_case name dro expected_exit)
    if(NOT EXISTS "${MITK_PET_IBSI_RESOLVED_DATA_DIR}/DRO/${dro}/PT")
      return()
    endif()
    _add_petsuv_cli_smoke(${name} ${expected_exit}
      "--input;${MITK_PET_IBSI_RESOLVED_DATA_DIR}/DRO/${dro}/PT;--output;${CMAKE_CURRENT_BINARY_DIR}/${name}_out.nrrd")
  endfunction()

  # 2 = MissingDICOMProperty against 6 = InvalidDICOMPropertyValue: the
  # distinction between "the tag is not there" and "the tag is there and
  # unusable", which decides whether re-exporting can help.
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit2_WeightAbsent
                                 DRO_error_2_0 2)
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit6_WeightZero
                                 DRO_error_2_1 6)

  # 3 = AmbiguousDecayTiming, here via the half-life gate on the
  # administration date: computed, this input yields a plausible but
  # wrong SUV, so its refusal is worth pinning at the process boundary
  # too.
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit3_UnrecoverableAdminDate
                                 DRO_error_4_1 3)

  # 11 and 12 give "this input is not convertible" its own codes, apart
  # from the catch-all 1 a caller reads as "MITK broke".
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit11_UnsupportedUnits
                                 DRO_error_2_7 11)
  _add_petsuv_cli_exit_code_case(MitkPETSUVCalculationCLI_Exit12_MissingPhilipsScale
                                 DRO_error_2_6 12)

  # The two Enhanced PET objects whose per-frame values vary, computed end
  # to end. A refusal would mean the reader's frame model is not reaching
  # the pipeline; a "rooted in a functional-group sequence" warning would
  # mean a tag of interest is registered relative to (5200,9229) or
  # (5200,9230), a shape that yields no property for a per-frame object.
  # The values themselves are pinned in-process by
  # mitkPETIBSIBenchmarkTest.
  foreach(_dro DRO_7_1_0 DRO_7_3_1)
    if(EXISTS "${MITK_PET_IBSI_RESOLVED_DATA_DIR}/DRO/${_dro}/PT")
      _add_petsuv_cli_smoke(MitkPETSUVCalculationCLI_EnhancedPerFrame_${_dro} 0
        "--input;${MITK_PET_IBSI_RESOLVED_DATA_DIR}/DRO/${_dro}/PT;--output;${CMAKE_CURRENT_BINARY_DIR}/MitkPETSUVCalculationCLI_${_dro}_out.nrrd;--variant;bw"
        REJECT_OUTPUT "rooted in a functional-group sequence")
    endif()
  endforeach()
endif()
