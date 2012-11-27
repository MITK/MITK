set(ENV_ATISTREAMSDKROOT $ENV{ATISTREAMSDKROOT})
if(ENV_ATISTREAMSDKROOT)
  find_path(
    OpenCL_INCLUDE_DIR
    NAMES CL/cl.h OpenCL/cl.h
    PATHS $ENV{ATISTREAMSDKROOT}/include
    NO_DEFAULT_PATH
    )

  if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(
        OpenCL_LIB_SEARCH_PATH
        ${OpenCL_LIB_SEARCH_PATH}
        $ENV{ATISTREAMSDKROOT}/lib/x86
        )
    else(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(
        OpenCL_LIB_SEARCH_PATH
        ${OpenCL_LIB_SEARCH_PATH}
        $ENV{ATISTREAMSDKROOT}/lib/x86_64
        )
    endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  endif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  find_library(
    OpenCL_LIBRARY
    NAMES OpenCL
    PATHS ${OpenCL_LIB_SEARCH_PATH}
    NO_DEFAULT_PATH
    )
else(ENV_ATISTREAMSDKROOT)
  find_path(
    OpenCL_INCLUDE_DIR
    NAMES CL/cl.h OpenCL/cl.h
    )

  find_library(
    OpenCL_LIBRARY
    NAMES OpenCL
    )
endif(ENV_ATISTREAMSDKROOT)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OpenCL
  DEFAULT_MSG
  OpenCL_LIBRARY OpenCL_INCLUDE_DIR
  )

if(OpenCL_FOUND)
  set(OpenCL_LIBRARIES ${OpenCL_LIBRARY})
else(OpenCL_FOUND)
  set(OpenCL_LIBRARIES)
endif(OpenCL_FOUND)

mark_as_advanced(
  OpenCL_INCLUDE_DIR
  OpenCL_LIBRARY
  )
