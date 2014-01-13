if(BUILD_TESTING)
if(GDCM_DIR)

# clear variables from prior files.cmake
# Else CMake would use the content of these variables and would try to create tests (which are not present in DICOMTesting).
set(MODULE_TESTS)
set(MODULE_IMAGE_TESTS)
set(MODULE_SURFACE_TESTS)
set(MODULE_TESTIMAGES)
set(MODULE_TESTSURFACES)
set(MODULE_CUSTOM_TESTS)
set(H_FILES)
set(CPP_FILES)

# now create a new module only for testing purposes
MITK_CREATE_MODULE( DICOMTesting
  DEPENDS DICOMReader
)

MITK_CHECK_MODULE(_RESULT DICOMTesting)
if(_RESULT)
  message(STATUS "DICOMTesting application won't be built. Missing: ${_RESULT}")
else(_RESULT)

# add helpful applications
MITK_USE_MODULE( DICOMTesting )
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${ALL_INCLUDE_DIRECTORIES})

# dumps out image information
add_executable(DumpDICOMMitkImage DumpDICOMMitkImage.cpp)
target_link_libraries(DumpDICOMMitkImage ${ALL_LIBRARIES})

# compares dumped out image information against reference dump
add_executable(VerifyDICOMMitkImageDump VerifyDICOMMitkImageDump.cpp)
target_link_libraries(VerifyDICOMMitkImageDump ${ALL_LIBRARIES})

add_subdirectory(Testing)
endif()

endif()
endif()
