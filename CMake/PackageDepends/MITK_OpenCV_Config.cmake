
FIND_PACKAGE(OpenCV REQUIRED)
LIST(APPEND ALL_LIBRARIES ${OpenCV_LIBS})
# adding release directory for fixing error with RelWitgDebInfo (debug dlls have the suffix "d". thus release dlls wont be loaded accidentally)
LINK_DIRECTORIES(${OpenCV_LIB_DIR} "${OpenCV_LIB_DIR}/Release")

IF(WIN32)
  OPTION(MITK_USE_videoInput "Use videoInput (DirectShow wrapper) library" OFF)
ENDIF(WIN32)