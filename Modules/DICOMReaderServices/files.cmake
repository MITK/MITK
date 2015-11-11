file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkBaseDICOMReaderService.cpp
  mitkAutoSelectingDICOMReaderService.cpp
  mitkClassicDICOMSeriesReaderService.cpp
  mitkDICOMReaderServicesActivator.cpp
  mitkDICOMFilesHelper.cpp
)
