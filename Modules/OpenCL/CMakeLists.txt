if(MITK_USE_OpenCL)
  find_package(OpenCL REQUIRED)
  include_directories(${OpenCL_INCLUDE_DIR})

  # on OS X the linker needs a flag pointing to the OpenCL.framework
  if(APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework OpenCL")
  endif(APPLE)

  # create the module
  MITK_CREATE_MODULE(MitkOcl
    INCLUDE_DIRS ${OpenCL_INCLUDE_DIR} ${MITK_BIN_DIR}
    DEPENDS Mitk
    PACKAGE_DEPENDS OpenCL
    ADDITIONAL_LIBS "${OpenCL_LIBRARY}")

  add_subdirectory(Testing)
endif(MITK_USE_OpenCL)
