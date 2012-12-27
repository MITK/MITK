#-----------------------------------------------------------------------------
# wrapper config
option(MITK_USE_Python "Build cswig Python wrapper support (requires CableSwig)." OFF)

#-----------------------------------------------------------------------------
# Do we need CableSwig?
set(MITK_NEED_CableSwig 0)

if(MITK_USE_Python)
  set(MITK_NEED_CableSwig 1)
endif(MITK_USE_Python)

if(MITK_NEED_CableSwig)

  if(NOT BUILD_SHARED_LIBS)
    message(FATAL_ERROR "Wrapping requires a shared build, change BUILD_SHARED_LIBS to ON")
  endif(NOT BUILD_SHARED_LIBS)

  # Search first if CableSwig is in the MITK source tree
  if(EXISTS ${MITK_SOURCE_DIR}/Utilities/CableSwig)
    set(CMAKE_MODULE_PATH ${MITK_SOURCE_DIR}/Utilities/CableSwig/SWIG/CMake)

    # CableSwig is included in the source distribution.
    set(MITK_BUILD_CABLESWIG 1)
    set(CableSwig_DIR ${MITK_BINARY_DIR}/Utilities/CableSwig CACHE PATH "CableSwig_DIR: The directory containing CableSwigConfig.cmake.")
    set(CableSwig_FOUND 1)
    set(CableSwig_INSTALL_ROOT ${MITK_INSTALL_LIB_DIR}/CSwig)
    include(${CableSwig_DIR}/CableSwigConfig.cmake OPTIONAL)
    subdirs(Utilities/CableSwig)
  else(EXISTS ${MITK_SOURCE_DIR}/Utilities/CableSwig)
    # If CableSwig is not in the source tree,
    # then try to find a binary build of CableSwig
    find_package(CableSwig)
    set(CMAKE_MODULE_PATH ${CableSwig_DIR}/SWIG/CMake)
  endif(EXISTS ${MITK_SOURCE_DIR}/Utilities/CableSwig)

  if(NOT CableSwig_FOUND)
    # We have not found CableSwig.  Complain.
    message(FATAL_ERROR "CableSwig is required for CSwig Wrapping.")
  endif(NOT CableSwig_FOUND)

endif(MITK_NEED_CableSwig)


