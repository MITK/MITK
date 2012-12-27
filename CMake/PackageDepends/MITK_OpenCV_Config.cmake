macro(REMOVE_LAST_PATH_ELEMENT_IF_EQUALS INPUT_VAR TARGET_VALUE)
  STRING( REPLACE "/" ";" INPUT_VARLIST ${${INPUT_VAR}} )
  LIST( LENGTH INPUT_VARLIST INPUT_VARLIST_LENGTH )

  SET(INPUT_VARLIST_LAST_INDEX ${INPUT_VARLIST_LENGTH})
  MATH( EXPR INPUT_VARLIST_LAST_INDEX "${INPUT_VARLIST_LENGTH}-1" )
  LIST( GET INPUT_VARLIST ${INPUT_VARLIST_LAST_INDEX} INPUT_VARLAST_ELEMENT )

  if(INPUT_VARLAST_ELEMENT MATCHES "^${TARGET_VALUE}$")
    LIST( REMOVE_AT INPUT_VARLIST ${INPUT_VARLIST_LAST_INDEX} )
  endif()

  set(NEW_VAR "")
  foreach(folderPart ${INPUT_VARLIST})
    if(NOT NEW_VAR)
      if(NOT WIN32)
        set(folderPart "/${folderPart}")
      endif()
      set(NEW_VAR "${folderPart}")
    else()
      set(NEW_VAR "${NEW_VAR}/${folderPart}")
    endif()
  endforeach()

  set(${INPUT_VAR} ${NEW_VAR})
endmacro()

# bug in opencv 2.4.2 and mitk: if we have a submodule OpenCV_LIB_DIR_OPT and
# OpenCV_LIB_DIR_DBG is cached when find_package is called
# resulting in wrong windows output paths, e.g. C:\OpenCV\lib\Release\Release
# removing last Release and Debug string manually to fix it
if( OpenCV_LIB_DIR )

  # remove the Release/Debug in <path>\Release and <path>\Debug
  # of OpenCV_LIB_DIR_OPT and OpenCV_LIB_DIR_DBG here
  REMOVE_LAST_PATH_ELEMENT_IF_EQUALS(OpenCV_LIB_DIR_DBG Debug)
  #message("OpenCV_LIB_DIR_DBG: ${OpenCV_LIB_DIR_DBG}")

  set(OpenCV_LIB_DIR_OPT ${OpenCV_LIB_DIR_DBG})
  #message("OpenCV_LIB_DIR_OPT: ${OpenCV_LIB_DIR_OPT}")

  REMOVE_LAST_PATH_ELEMENT_IF_EQUALS(OpenCV_3RDPARTY_LIB_DIR_DBG Debug)
  #message("OpenCV_3RDPARTY_LIB_DIR_DBG: ${OpenCV_3RDPARTY_LIB_DIR_DBG}")

  set(OpenCV_3RDPARTY_LIB_DIR_OPT ${OpenCV_3RDPARTY_LIB_DIR_DBG})
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
