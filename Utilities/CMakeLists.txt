SUPPRESS_ALL_WARNINGS()

# most stuff of these uses itk_zlib.h (via mitkIpPic.h)
find_package(ITK)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1)
include(${ITK_USE_FILE})

# some legacy util files include in the old style with prefixed directory,
# like #include <ipPic/mitkIpPic.h>
include_directories(.)

add_subdirectory(IpPic)
add_subdirectory(IpFunc)
add_subdirectory(IpSegmentation)

set(MITK_MODULE_NAME_REGEX_MATCH )
set(MITK_MODULE_NAME_REGEX_NOT_MATCH )
set(MITK_MODULE_NAME_PREFIX )
add_subdirectory(mbilog)
add_subdirectory(qtsingleapplication)
