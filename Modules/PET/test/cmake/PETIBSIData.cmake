# PETIBSIData.cmake
#
# Acquires the IBSI-SUV digital reference object (DRO) tree from the
# oncoray/suv_computation benchmark for use by the PET regression tests.
#
# The data is licensed CC BY 4.0 by the IBSI / Image Biomarker
# Standardisation Initiative (Vácha, Zwanenburg et al.) and is fetched
# verbatim from upstream -- we do not redistribute.
#
# Configure-time variables (cache):
#   MITK_PET_IBSI_DATA_DIR       Path to a pre-existing checkout. If set,
#                                the download is skipped and the directory
#                                is validated. Useful for offline / proxied
#                                builds and for development against a local
#                                fork.
#   MITK_PET_DOWNLOAD_IBSI_DATA  Master switch (default ON). When OFF and
#                                MITK_PET_IBSI_DATA_DIR is unset, the IBSI
#                                tests skip with exit 77 at runtime.
#   MITK_PET_IBSI_DATA_GIT_TAG   Commit SHA pinned for reproducibility.
#                                Bump deliberately and verify the
#                                BenchmarkCase manifest still matches.
#
# Output:
#   MITK_PET_IBSI_DATA_DIR (cache)  Resolved path to the DRO tree (if any).

set(MITK_PET_DOWNLOAD_IBSI_DATA ON CACHE BOOL
  "Download the IBSI suv_computation DRO tree at configure time when no \
MITK_PET_IBSI_DATA_DIR is provided. The data is needed by the PET regression \
test suite (mitkPETIBSIBenchmarkTest); when neither this option is ON nor \
MITK_PET_IBSI_DATA_DIR points at a checkout, the test skips at runtime.")

set(MITK_PET_IBSI_DATA_GIT_TAG "381583521b877a441450e14efc62e07e59e4bef0"
  CACHE STRING "Pinned commit SHA of oncoray/suv_computation. Bumping this \
SHA may invalidate the BenchmarkCase manifest in mitkPETIBSIBenchmarkTest.cpp; \
re-validate after every bump.")
mark_as_advanced(MITK_PET_IBSI_DATA_GIT_TAG)

set(MITK_PET_IBSI_DATA_DIR "" CACHE PATH
  "Optional pre-existing checkout of oncoray/suv_computation. When set, the \
PET regression test suite uses this path verbatim and the configure-time \
download is skipped.")

# Helper: validate a candidate data dir by probing for known content.
# docs/DRO_list.csv identifies the tree as an IBSI-SUV checkout at all;
# DRO/DRO_7_0_0 identifies it as v3.0.1 or newer. Without the second probe
# a stale checkout configures cleanly and then fails at test time as a
# couple of dozen unexplained "directory not found" cases.
function(_petibsidata_validate_dir candidate result_var)
  set(${result_var} FALSE PARENT_SCOPE)
  if(NOT EXISTS "${candidate}/docs/DRO_list.csv")
    return()
  endif()
  if(NOT EXISTS "${candidate}/DRO/DRO_7_0_0/PT")
    return()
  endif()
  set(${result_var} TRUE PARENT_SCOPE)
endfunction()

if(MITK_PET_IBSI_DATA_DIR)
  _petibsidata_validate_dir("${MITK_PET_IBSI_DATA_DIR}" _ibsi_valid)
  if(NOT _ibsi_valid)
    message(FATAL_ERROR
      "MITK_PET_IBSI_DATA_DIR='${MITK_PET_IBSI_DATA_DIR}' does not contain a \
recognizable IBSI suv_computation tree at v3.0.1 or newer (expected \
docs/DRO_list.csv and DRO/DRO_7_0_0/PT). Provide a correct path or unset \
to fall back to the FetchContent download.")
  endif()
  message(STATUS "PET IBSI DRO tree: using user-supplied ${MITK_PET_IBSI_DATA_DIR}")
elseif(MITK_PET_DOWNLOAD_IBSI_DATA)
  include(FetchContent)
  message(STATUS "PET IBSI DRO tree: fetching oncoray/suv_computation @ ${MITK_PET_IBSI_DATA_GIT_TAG}")
  FetchContent_Declare(PETIBSIData
    GIT_REPOSITORY https://github.com/oncoray/suv_computation.git
    GIT_TAG        ${MITK_PET_IBSI_DATA_GIT_TAG}
    GIT_SHALLOW    FALSE  # SHA pin requires full history fetch
    SOURCE_DIR     ${CMAKE_BINARY_DIR}/PETIBSIData-src
  )
  # The upstream repo has no CMakeLists.txt, so MakeAvailable populates
  # the source tree without invoking add_subdirectory(). The fetched
  # tree is data-only and is consumed by tests at runtime via the
  # MITK_PET_IBSI_DATA_DIR cache variable.
  FetchContent_MakeAvailable(PETIBSIData)
  _petibsidata_validate_dir("${petibsidata_SOURCE_DIR}" _ibsi_valid)
  if(NOT _ibsi_valid)
    message(FATAL_ERROR
      "FetchContent populated PETIBSIData at '${petibsidata_SOURCE_DIR}' but \
docs/DRO_list.csv or DRO/DRO_7_0_0/PT is missing. The pinned SHA may be \
wrong or the upstream repository layout has changed.")
  endif()
  set(MITK_PET_IBSI_DATA_DIR "${petibsidata_SOURCE_DIR}" CACHE PATH "" FORCE)
  message(STATUS "PET IBSI DRO tree: ${MITK_PET_IBSI_DATA_DIR}")
else()
  message(STATUS "PET IBSI DRO tree: not configured. mitkPETIBSIBenchmarkTest \
will skip at runtime. Enable MITK_PET_DOWNLOAD_IBSI_DATA or set \
MITK_PET_IBSI_DATA_DIR to opt in.")
endif()
