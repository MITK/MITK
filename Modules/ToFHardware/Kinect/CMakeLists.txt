#Begin Kinect hardware
OPTION(MITK_USE_TOF_KINECT "Enable support for Kinect camera" OFF)

IF(MITK_USE_TOF_KINECT)
  mitkFunctionCheckCompilerFlags("-Wno-error=unknown-pragmas" CMAKE_CXX_FLAGS)
  mitkFunctionCheckCompilerFlags("-Wno-error=reorder" CMAKE_CXX_FLAGS)

  FIND_LIBRARY(MITK_KINECT_LIB libOpenNI DOC "Kinect access library.")
  FIND_PATH(MITK_KINECT_INCLUDE_DIR XnCppWrapper.h DOC  "Include directory of Kinect camera.")
  SET(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},Microsoft Kinect)
  SET(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_KINECT_INCLUDE_DIR})
  SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_KINECT_LIB})

  MITK_CREATE_MODULE(
    INCLUDE_DIRS
      PUBLIC ${MITK_BIN_DIR}
      PRIVATE ${INCLUDE_DIRS_INTERNAL}
    DEPENDS MitkToFHardware
    ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
    AUTOLOAD_WITH MitkToFHardware
  )
  Message("Kinect Module generated")
  add_subdirectory(Testing)
ENDIF(MITK_USE_TOF_KINECT)
#End Kinect Hardware
