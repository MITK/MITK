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
#                                fork. Never written by this script.
#   MITK_PET_DOWNLOAD_IBSI_DATA  Master switch (default ON). When OFF and
#                                MITK_PET_IBSI_DATA_DIR is unset, the IBSI
#                                tests skip with exit 77 at runtime.
#
# Output:
#   MITK_PET_IBSI_RESOLVED_DATA_DIR  Path to the DRO tree the tests use, or
#                                    empty when there is none.

set(MITK_PET_DOWNLOAD_IBSI_DATA ON CACHE BOOL
  "Download the IBSI suv_computation DRO tree at configure time when no \
MITK_PET_IBSI_DATA_DIR is provided. The data is needed by the PET regression \
test suite (mitkPETIBSIBenchmarkTest); when neither this option is ON nor \
MITK_PET_IBSI_DATA_DIR points at a checkout, the test skips at runtime.")

# The pin is part of the test code, not of the build configuration: the
# BenchmarkCase manifest in mitkPETIBSIBenchmarkTest.cpp is only valid for
# this commit. A cache variable would keep an existing build tree on the
# previous commit after a bump.
set(MITK_PET_IBSI_DATA_GIT_TAG "381583521b877a441450e14efc62e07e59e4bef0")
unset(MITK_PET_IBSI_DATA_GIT_TAG CACHE)

set(MITK_PET_IBSI_DATA_DIR "" CACHE PATH
  "Optional pre-existing checkout of oncoray/suv_computation. When set, the \
PET regression test suite uses this path verbatim and the configure-time \
download is skipped.")

set(_petibsidata_download_dir "${CMAKE_BINARY_DIR}/PETIBSIData-src")

# Older versions of this script stored the download location in
# MITK_PET_IBSI_DATA_DIR. Taken as user input, it would skip the download
# and with it every pin bump, so it is dropped in favour of a fresh fetch.
if(MITK_PET_IBSI_DATA_DIR)
  cmake_path(COMPARE "${MITK_PET_IBSI_DATA_DIR}" EQUAL "${_petibsidata_download_dir}" _ibsi_is_download_dir)
  if(_ibsi_is_download_dir)
    set(MITK_PET_IBSI_DATA_DIR "" CACHE PATH "" FORCE)
  endif()
endif()

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

set(MITK_PET_IBSI_RESOLVED_DATA_DIR "")

if(MITK_PET_IBSI_DATA_DIR)
  _petibsidata_validate_dir("${MITK_PET_IBSI_DATA_DIR}" _ibsi_valid)
  if(NOT _ibsi_valid)
    message(FATAL_ERROR
      "MITK_PET_IBSI_DATA_DIR='${MITK_PET_IBSI_DATA_DIR}' does not contain a \
recognizable IBSI suv_computation tree at v3.0.1 or newer (expected \
docs/DRO_list.csv and DRO/DRO_7_0_0/PT). Provide a correct path or unset \
to fall back to the FetchContent download.")
  endif()
  set(MITK_PET_IBSI_RESOLVED_DATA_DIR "${MITK_PET_IBSI_DATA_DIR}")
  message(STATUS "PET IBSI DRO tree: using user-supplied ${MITK_PET_IBSI_DATA_DIR}")
elseif(MITK_PET_DOWNLOAD_IBSI_DATA)
  include(FetchContent)
  message(STATUS "PET IBSI DRO tree: fetching oncoray/suv_computation @ ${MITK_PET_IBSI_DATA_GIT_TAG}")
  FetchContent_Declare(PETIBSIData
    GIT_REPOSITORY https://github.com/oncoray/suv_computation.git
    GIT_TAG        ${MITK_PET_IBSI_DATA_GIT_TAG}
    GIT_SHALLOW    FALSE  # SHA pin requires full history fetch
    SOURCE_DIR     ${_petibsidata_download_dir}
  )
  # The upstream repo has no CMakeLists.txt, so MakeAvailable populates
  # the source tree without invoking add_subdirectory(). The fetched
  # tree is data-only and is consumed by tests at runtime via
  # MITK_PET_IBSI_RESOLVED_DATA_DIR.
  FetchContent_MakeAvailable(PETIBSIData)
  _petibsidata_validate_dir("${petibsidata_SOURCE_DIR}" _ibsi_valid)
  if(NOT _ibsi_valid)
    message(FATAL_ERROR
      "FetchContent populated PETIBSIData at '${petibsidata_SOURCE_DIR}' but \
docs/DRO_list.csv or DRO/DRO_7_0_0/PT is missing. The pinned SHA may be \
wrong or the upstream repository layout has changed.")
  endif()
  set(MITK_PET_IBSI_RESOLVED_DATA_DIR "${petibsidata_SOURCE_DIR}")
  message(STATUS "PET IBSI DRO tree: ${MITK_PET_IBSI_RESOLVED_DATA_DIR}")
else()
  message(STATUS "PET IBSI DRO tree: not configured. mitkPETIBSIBenchmarkTest \
will skip at runtime. Enable MITK_PET_DOWNLOAD_IBSI_DATA or set \
MITK_PET_IBSI_DATA_DIR to opt in.")
endif()
