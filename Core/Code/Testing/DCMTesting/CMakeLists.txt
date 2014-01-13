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
MITK_CREATE_MODULE( mitkDCMTesting
  DEPENDS Mitk # Mitk.so
)

MITK_CHECK_MODULE(_RESULT mitkDCMTesting)
if(_RESULT)
  message(STATUS "mitkDCMTesting application won't be built. Missing: ${_RESULT}")
else(_RESULT)

# add helpful applications
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# remove with newer MITK version
MITK_USE_MODULE( mitkDCMTesting )
include_directories(${ALL_INCLUDE_DIRECTORIES})

# dumps out image information
add_executable(DumpDCMMitkImage DumpDCMMitkImage.cpp)
#Newer code: mitk_use_modules(TARGET DumpDCMMitkImage MODULES mitkDCMTesting)
target_link_libraries(DumpDCMMitkImage ${ALL_LIBRARIES})

# compares dumped out image information against reference dump
add_executable(VerifyDCMMitkImageDump VerifyDCMMitkImageDump.cpp)
#Newer code: mitk_use_modules(TARGET VerifyDCMMitkImageDump MODULES mitkDCMTesting)
target_link_libraries(VerifyDCMMitkImageDump ${ALL_LIBRARIES})

add_subdirectory(Testing)
endif()

endif()
endif()
