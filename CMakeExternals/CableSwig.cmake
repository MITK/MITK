#-----------------------------------------------------------------------------
# CableSwig
#-----------------------------------------------------------------------------

IF(MITK_USE_Python)

# Sanity checks
IF(DEFINED CableSwig_DIR AND NOT EXISTS ${CableSwig_DIR})
  MESSAGE(FATAL_ERROR "CableSwig_DIR variable is defined but corresponds to non-existing directory")
ENDIF()

SET(proj CableSwig)
SET(proj_DEPENDENCIES )
SET(CableSwig_DEPENDS ${proj})

IF(NOT DEFINED CableSwig_DIR)

  SET(additional_cmake_args )

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/CableSwig-ITK-3.20.0.tar.gz
     URL_MD5 893882bf8b4fbfbae3fe8c747a75f7a0     
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       -DBUILD_TESTING:BOOL=OFF
       -DSWIG_BUILD_EXAMPLES:BOOL=OFF
     DEPENDS ${proj_DEPENDENCIES}
    )

  SET(CableSwig_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

ELSE()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

ENDIF()

ENDIF()
