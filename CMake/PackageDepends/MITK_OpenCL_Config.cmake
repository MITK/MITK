find_package(OpenCL REQUIRED)
list(APPEND ALL_LIBRARIES ${OPENCL_LIBRARIES})
list(APPEND ALL_INCLUDE_DIRECTORIES ${OPENCL_INCLUDE_DIRS})

# on OS X the linker needs a flag pointing to the OpenCL.framework
if(APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework OpenCL")
endif(APPLE)
