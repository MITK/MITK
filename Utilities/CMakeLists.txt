SUPPRESS_ALL_WARNINGS()

# most stuff of these uses itk_zlib.h (via mitkIpPic.h)
find_package(ITK)
include(${ITK_USE_FILE})

# some legacy util files include in the old style with prefixed directory,
# like #include <ipPic/mitkIpPic.h>
include_directories(.)

subdirs(
  ipPic
  ipFunc
  ipSegmentation
  mbilog
  qtsingleapplication
  KWStyle
)

# mbilog is independent of mitk, and cant use mitk macros
# configuring happens through mbilog/mbilogConfig.cmake.in
set(mbilog_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/mbilog" "${CMAKE_CURRENT_BINARY_DIR}/mbilog")
set(mbilog_CONFIG_FILE "${PROJECT_BINARY_DIR}/${MODULES_CONF_DIRNAME}/mbilogConfig.cmake" CACHE INTERNAL "Path to module config" FORCE)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mbilog/mbilogConfig.cmake.in" "${mbilog_CONFIG_FILE}")
