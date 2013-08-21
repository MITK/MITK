find_package(ITK REQUIRED)
#
# for some reason this does not work on windows, probably an ITK bug
# ITK_BUILD_SHARED is OFF even in shared library builds
#
#if(ITK_FOUND AND NOT ITK_BUILD_SHARED)
#  message(FATAL_ERROR "MITK only supports a ITK which was built with shared libraries. Turn on BUILD_SHARED_LIBS in your ITK config.")
#endif(ITK_FOUND AND NOT ITK_BUILD_SHARED)

set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1)

include(${ITK_USE_FILE})
list(APPEND ALL_LIBRARIES ${ITK_LIBRARIES})
list(APPEND ALL_INCLUDE_DIRECTORIES ${ITK_INCLUDE_DIRS})

find_package(GDCM PATHS ${ITK_GDCM_DIR} REQUIRED)
list(APPEND ALL_INCLUDE_DIRECTORIES ${GDCM_INCLUDE_DIRS})
list(APPEND ALL_LIBRARIES ${GDCM_LIBRARIES})
include(${GDCM_USE_FILE})
