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

SUBDIRS(ann ipSegmentation IIL4MITK pic2vtk tinyxml Poco qwt)
# IF(NOT MITK_CHILI_PLUGIN)
SUBDIRS(ipPic ipFunc)

# ENDIF(NOT MITK_CHILI_PLUGIN)

SET(Poco_INCLUDE_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}/Poco 
    CACHE PATH 
    "top-level directory containing the poco include directories. E.g /usr/local/include/ or c:\\poco\\include\\poco-1.3.2"
)

SET(Poco_LIBRARY_DIR 
    ${LIBRARY_OUTPUT_PATH}
    CACHE PATH 
    "top-level directory containing the poco libraries."
)

OPTION(MITK_USE_OPENCHERRY "Build the openCherry platform" OFF)

IF(MITK_USE_OPENCHERRY)       
  ADD_SUBDIRECTORY(openCherry)
  SET(openCherry_DIR ${CMAKE_CURRENT_BINARY_DIR}/openCherry CACHE PATH "The directory containing a CMake configuration file for openCherry" FORCE)
ENDIF(MITK_USE_OPENCHERRY)

