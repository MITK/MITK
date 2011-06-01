#-----------------------------------------------------------------------------
# wrapper config
OPTION(MITK_USE_Python "Build cswig Python wrapper support (requires CableSwig)." OFF)

#-----------------------------------------------------------------------------
# Do we need CableSwig?
SET(MITK_NEED_CableSwig 0)

IF(MITK_USE_Python)
  SET(MITK_NEED_CableSwig 1)
ENDIF(MITK_USE_Python)

IF(MITK_NEED_CableSwig)

  IF(NOT BUILD_SHARED_LIBS)
    MESSAGE(FATAL_ERROR "Wrapping requires a shared build, change BUILD_SHARED_LIBS to ON")
  ENDIF(NOT BUILD_SHARED_LIBS)

  # Search first if CableSwig is in the MITK source tree
  IF(EXISTS ${MITK_SOURCE_DIR}/Utilities/CableSwig)
    SET(CMAKE_MODULE_PATH ${MITK_SOURCE_DIR}/Utilities/CableSwig/SWIG/CMake)

    # CableSwig is included in the source distribution.
    SET(MITK_BUILD_CABLESWIG 1)
    SET(CableSwig_DIR ${MITK_BINARY_DIR}/Utilities/CableSwig CACHE PATH "CableSwig_DIR: The directory containing CableSwigConfig.cmake.")
    SET(CableSwig_FOUND 1)
    SET(CableSwig_INSTALL_ROOT ${MITK_INSTALL_LIB_DIR}/CSwig)
    INCLUDE(${CableSwig_DIR}/CableSwigConfig.cmake OPTIONAL) 
    SUBDIRS(Utilities/CableSwig)
  ELSE(EXISTS ${MITK_SOURCE_DIR}/Utilities/CableSwig)
    # If CableSwig is not in the source tree, 
    # then try to find a binary build of CableSwig
    FIND_PACKAGE(CableSwig)
    SET(CMAKE_MODULE_PATH ${CableSwig_DIR}/SWIG/CMake)
  ENDIF(EXISTS ${MITK_SOURCE_DIR}/Utilities/CableSwig)

  IF(NOT CableSwig_FOUND)
    # We have not found CableSwig.  Complain.
    MESSAGE(FATAL_ERROR "CableSwig is required for CSwig Wrapping.")
  ENDIF(NOT CableSwig_FOUND)

ENDIF(MITK_NEED_CableSwig)


