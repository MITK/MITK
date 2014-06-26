#
#  IMPORTANT NOTE:
#
#  This module is a testing module for the DicomSeriesReader class,
#  which is meant to be replaced by the Module DCMReader.
#  To have the "old" code tested before it is removed, we still
#  keep this DCMTesting module.
#
#  Note that the DCMTesting module is practically a copy of
#  this DCMTesting module, doing the same kind of testing, just
#  for the new readers instead of DicomSeriesReader.
#

if(BUILD_TESTING)
if(GDCM_DIR)

# clear variables from prior files.cmake
# Else CMake would use the content of these variables and would try to create tests (which are not present in DCMTesting).
set(MODULE_TESTS)
set(MODULE_IMAGE_TESTS)
set(MODULE_SURFACE_TESTS)
set(MODULE_TESTIMAGES)
set(MODULE_TESTSURFACES)
set(MODULE_CUSTOM_TESTS)
set(H_FILES)
set(CPP_FILES)

# now create a new module only for testing purposes
MITK_CREATE_MODULE(
  DEPENDS MitkCore # Mitk.so
)

mitk_check_module_dependencies(MODULES MitkDCMTesting MISSING_DEPENDENCIES_VAR _missing_deps)

if(_missing_deps)
  message(STATUS "mitkDCMTesting application won't be built. Missing: ${_missing_deps}")
else(_missing_deps)

  # add helpful applications
  include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

  # dumps out image information
  add_executable(DumpDCMMitkImage DumpDCMMitkImage.cpp)
  mitk_use_modules(TARGET DumpDCMMitkImage MODULES MitkDCMTesting)

  # compares dumped out image information against reference dump
  add_executable(VerifyDCMMitkImageDump VerifyDCMMitkImageDump.cpp)
  mitk_use_modules(TARGET VerifyDCMMitkImageDump MODULES MitkDCMTesting)

  add_subdirectory(Testing)
endif()

endif()
endif()
