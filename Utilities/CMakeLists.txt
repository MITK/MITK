SUPPRESS_ALL_WARNINGS()

# most stuff of these uses itk_zlib.h (via mitkIpPic.h)
find_package(ITK)
include(${ITK_USE_FILE})

# some legacy util files include in the old style with prefixed directory, 
# like #include <ipPic/mitkIpPic.h>
include_directories(.)

subdirs(
  ann
  ipSegmentation
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
set(mbilog_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/mbilog" "${CMAKE_CURRENT_BINARY_DIR}/mbilog")
set(mbilog_CONFIG_FILE "${PROJECT_BINARY_DIR}/${MODULES_CONF_DIRNAME}/mbilogConfig.cmake" CACHE INTERNAL "Path to module config" FORCE)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mbilog/mbilogConfig.cmake.in" "${mbilog_CONFIG_FILE}")

# if(NOT MITK_CHILI_PLUGIN)
subdirs(ipPic ipFunc)
add_subdirectory(KWStyle)
# endif(NOT MITK_CHILI_PLUGIN)

set(Poco_INCLUDE_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}/Poco 
    CACHE PATH 
    "top-level directory containing the poco include directories. E.g /usr/local/include/ or c:\\poco\\include\\poco-1.3.2"
)

set(Poco_LIBRARY_DIR 
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    CACHE PATH 
    "top-level directory containing the poco libraries."
)


