SUPPRESS_ALL_WARNINGS()

# most stuff of these uses itk_zlib.h (via mitkIpPic.h)
FIND_PACKAGE(ITK)
INCLUDE(${ITK_USE_FILE})

# some legacy util files include in the old style with prefixed directory, 
# like #include <ipPic/mitkIpPic.h>
INCLUDE_DIRECTORIES(.)

SUBDIRS(
  ann
  ipSegmentation
  IIL4MITK
  pic2vtk
  tinyxml
  Poco
  qwt
  qxt
  mbilog
  glew
  vecmath
  qtsingleapplication
)

# mbilog is independent of mitk, and cant use mitk macros
# configuring happens through mbilog/mbilogConfig.cmake.in
SET(mbilog_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/mbilog" "${CMAKE_CURRENT_BINARY_DIR}/mbilog")
SET(mbilog_CONFIG_FILE "${PROJECT_BINARY_DIR}/${MODULES_CONF_DIRNAME}/mbilogConfig.cmake" CACHE INTERNAL "Path to module config" FORCE)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/mbilog/mbilogConfig.cmake.in" "${mbilog_CONFIG_FILE}")

# IF(NOT MITK_CHILI_PLUGIN)
SUBDIRS(ipPic ipFunc)
ADD_SUBDIRECTORY(KWStyle)
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


