file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

SET(CPP_FILES
  mitkRTDoseReader.cpp
  mitkRTStructureSetReader.cpp
  mitkRTConstants.cpp
  mitkIsoDoseLevel.cpp
  mitkIsoDoseLevelCollections.cpp
  mitkIsoDoseLevelSetProperty.cpp
  mitkIsoDoseLevelVectorProperty.cpp
)

