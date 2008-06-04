INCLUDE(VTK_ITK.cmake)
SUPPRESS_VC8_DEPRECATED_WARNINGS()
# INCLUDE_DIRECTORIES(${${KITNAME}_INCLUDE_DIRS})

# most stuff of these uses itk_zlib.h (via ipPic.h)
FIND_PACKAGE(ITK)
INCLUDE_DIRECTORIES(${ITK_INCLUDE_DIRS})
LINK_DIRECTORIES(${ITK_LIBRARY_DIRS})

# some legacy util files include in the old style with prefixed directory, 
# like #include <ipPic/ipPic.h>
INCLUDE_DIRECTORIES(.)

SUBDIRS(ann ipSegmentation IIL4MITK pic2vtk tinyxml)
# IF(NOT MITK_CHILI_PLUGIN)
SUBDIRS(ipPic ipFunc)

# ENDIF(NOT MITK_CHILI_PLUGIN)

IF(DESIRED_QT_VERSION EQUAL 4)
  OPTION(BUILD_CHERRY "EXPERIMENTAL! Build the openCherry platform" OFF)
ENDIF(DESIRED_QT_VERSION EQUAL 4)

IF(BUILD_CHERRY)
  SET(OPENCHERRY_PLUGIN_DIRS ${CMAKE_CURRENT_BINARY_DIR}/../Q4Plugins
      CACHE STRING "A comma-separated list of directories containing plugins")
         
  ADD_SUBDIRECTORY(openCherry)
ENDIF(BUILD_CHERRY)

