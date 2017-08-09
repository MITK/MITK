file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkAutoSelectingDICOMReaderService.cpp
  mitkClassicDICOMSeriesReaderService.cpp
  mitkDICOMReaderServicesActivator.cpp
  mitkDICOMTagsOfInterestService.cpp
)
