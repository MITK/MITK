list(APPEND ALL_LIBRARIES ${OpenCV_LIBS})
list(APPEND ALL_INCLUDE_DIRECTORIES ${OpenCV_INCLUDE_DIRS})

# adding option for videoinput library on windows (for directshow based frame grabbing)
if(WIN32)
  option(MITK_USE_videoInput "Use videoInput (DirectShow wrapper) library" OFF)
endif(WIN32)
