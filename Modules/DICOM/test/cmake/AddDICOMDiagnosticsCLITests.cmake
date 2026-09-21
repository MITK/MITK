# AddDICOMDiagnosticsCLITests.cmake
#
# Registers ctest cases for the DICOMVolumeDiagnostics CLI app, covering the
# frame-model facts it reports. Only conditions reachable from MITK-Data are
# registered here; the detection logic itself is unit tested by
# mitkDICOMFrameLayoutTest, and the conditions that need a generated file are
# checked by hand, because the files exist only inside the test process while
# the CLI is a separate executable.

# The app is optional: Modules/DICOM/cmdapps/ creates the target only when
# BUILD_DICOMCmdApps or MITK_BUILD_ALL_APPS is set. Skip rather than emit a
# generator expression that cannot resolve.
if(NOT TARGET MitkDICOMVolumeDiagnostics)
  return()
endif()

set(_assert_script "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AssertJsonContains.cmake")
set(_cli_exe       "$<TARGET_FILE:MitkDICOMVolumeDiagnostics>")

# Mirror the runtime PATH wiring the module-test macro applies so the freshly
# built executable finds its DLLs on Windows.
mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELEASE release RELEASE)
mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEBUG   debug   DEBUG)
set(_diag_path ${MITK_RUNTIME_PATH_RELEASE} ${MITK_RUNTIME_PATH_DEBUG} $ENV{PATH})
list(REMOVE_DUPLICATES _diag_path)
string(REGEX REPLACE "\;" "\\\;" _diag_path "${_diag_path}")

# RT/Dose/RD.dcm is a 263-frame RT Dose object without functional groups: the
# only input in MITK-Data that produces a frame-model finding.
set(_rd_dose "${MITK_DATA_DIR}/RT/Dose/RD.dcm")

if(EXISTS "${_rd_dose}")
  set(_report "${CMAKE_CURRENT_BINARY_DIR}/mitkDICOMVolumeDiagnosticsCLI_RTDose.json")

  add_test(NAME mitkDICOMVolumeDiagnosticsCLI_RTDose_NoPerFrameMetadata
    COMMAND ${CMAKE_COMMAND}
            -DCMD=${_cli_exe}
            "-DARGS=-i;${_rd_dose};-o;${_report}"
            -DJSON=${_report}
            # The report file is dumped compact, without the spaces the
            # pretty-printed stdout copy has.
            "-DCONTAINS=\"type\":\"no_per_frame_metadata\";\"frame_count\":263;\"frame_model\":false"
            -P "${_assert_script}")

  set_property(TEST mitkDICOMVolumeDiagnosticsCLI_RTDose_NoPerFrameMetadata
               APPEND PROPERTY ENVIRONMENT "PATH=${_diag_path}")
  set_property(TEST mitkDICOMVolumeDiagnosticsCLI_RTDose_NoPerFrameMetadata
               PROPERTY LABELS "DICOM" "DICOMDiagnosticsCLI")
endif()
