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

SUBDIRS(ann ipSegmentation IIL4MITK pic2vtk tinyxml Poco qwt mbilog)

# mbilog is independent of mitk, and cant use mitk macros
# configuring happens through mbilog/mbilogConfig.cmake.in
SET(mbilog_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/mbilog")
IF(MBILOG_ENABLE_DEBUG_MESSAGES)
  SET(MBILOG_DEFINITIONS "-DMBILOG_ENABLE_DEBUG")
ENDIF(MBILOG_ENABLE_DEBUG_MESSAGES)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/mbilog/mbilogConfig.cmake.in"
"${MITK_MODULES_CONF_DIR}/mbilogConfig.cmake")

# IF(NOT MITK_CHILI_PLUGIN)
SUBDIRS(ipPic ipFunc)

# ENDIF(NOT MITK_CHILI_PLUGIN)

SET(Poco_INCLUDE_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}/Poco 
    CACHE PATH 
    "top-level directory containing the poco include directories. E.g /usr/local/include/ or c:\\poco\\include\\poco-1.3.2"
)

SET(Poco_LIBRARY_DIR 
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    CACHE PATH 
    "top-level directory containing the poco libraries."
)


