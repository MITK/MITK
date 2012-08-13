# bug in opencv 2.4.2 and mitk: if we have a submodule OpenCV_LIB_DIR_OPT and
# OpenCV_LIB_DIR_DBG is cached when find_package is called
# resulting in wrong windows output paths, e.g. C:\OpenCV\lib\Release\Release
# removing last Release and Debug string manually to fix it
if( OpenCV_LIB_DIR )

  string(REPLACE "/" ";" OpenCV_LIB_DIR_OPT_LIST ${OpenCV_LIB_DIR_OPT})
  set(OpenCV_LIB_DIR_OPT "")
  foreach(folderPart ${OpenCV_LIB_DIR_OPT_LIST})
    if(NOT folderPart MATCHES  "Release")
    if(NOT OpenCV_LIB_DIR_OPT)
      set(OpenCV_LIB_DIR_OPT "${folderPart}")
    else()
      set(OpenCV_LIB_DIR_OPT "${OpenCV_LIB_DIR_OPT}/${folderPart}")
    endif()
    endif()
  endforeach()
  set(OpenCV_LIB_DIR_DBG ${OpenCV_LIB_DIR_OPT})

  string(REPLACE "/" ";" OpenCV_3RDPARTY_LIB_DIR_OPT_LIST ${OpenCV_3RDPARTY_LIB_DIR_OPT})
  set(OpenCV_3RDPARTY_LIB_DIR_OPT "")
  foreach(folderPart ${OpenCV_3RDPARTY_LIB_DIR_OPT_LIST})
    if(NOT folderPart MATCHES  "Release")
    if(NOT OpenCV_3RDPARTY_LIB_DIR_OPT)
      set(OpenCV_3RDPARTY_LIB_DIR_OPT "${folderPart}")
    else()
      set(OpenCV_3RDPARTY_LIB_DIR_OPT "${OpenCV_3RDPARTY_LIB_DIR_OPT}/${folderPart}")
    endif()
    endif()
  endforeach()
  set(OpenCV_3RDPARTY_LIB_DIR_DBG ${OpenCV_3RDPARTY_LIB_DIR_OPT})
  
  #message("OpenCV_LIB_DIR_OPT: ${OpenCV_LIB_DIR_OPT}")
  #message("OpenCV_LIB_DIR_DBG: ${OpenCV_LIB_DIR_DBG}")
  #message("OpenCV_3RDPARTY_LIB_DIR_DBG: ${OpenCV_3RDPARTY_LIB_DIR_DBG}")
  #message("OpenCV_3RDPARTY_LIB_DIR_OPT: ${OpenCV_3RDPARTY_LIB_DIR_OPT}")
  
endif()

find_package(OpenCV REQUIRED)
list(APPEND ALL_LIBRARIES ${OpenCV_LIBS})

# adding release directory for fixing error with RelWitgDebInfo (debug dlls have the suffix "d". thus release dlls wont be loaded accidentally)
link_directories(${OpenCV_LIB_DIR} "${OpenCV_LIB_DIR}/Release")

# adding option for videoinput library on windows (for directshow based frame grabbing)  
if(WIN32)
  option(MITK_USE_videoInput "Use videoInput (DirectShow wrapper) library" OFF)
endif(WIN32)