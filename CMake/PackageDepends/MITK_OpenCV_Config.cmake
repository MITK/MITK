
find_package(OpenCV REQUIRED)
list(APPEND ALL_LIBRARIES ${OpenCV_LIBS})
# adding release directory for fixing error with RelWitgDebInfo (debug dlls have the suffix "d". thus release dlls wont be loaded accidentally)
link_directories(${OpenCV_LIB_DIR} "${OpenCV_LIB_DIR}/Release")

if(WIN32)
  option(MITK_USE_videoInput "Use videoInput (DirectShow wrapper) library" OFF)
endif(WIN32)