file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

SET(CPP_FILES
  mitkRTConstants.cpp
  mitkIsoDoseLevel.cpp
  mitkIsoDoseLevelCollections.cpp
  mitkIsoDoseLevelSetProperty.cpp
  mitkIsoDoseLevelVectorProperty.cpp
  mitkDoseImageVtkMapper2D.cpp
  mitkIsoLevelsGenerator.cpp
  mitkDoseNodeHelper.cpp
  mitkDICOMRTMimeTypes.cpp
)

set(TPP_FILES
)

set(MOC_H_FILES
)
