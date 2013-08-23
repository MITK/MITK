#-----------------------------------------------------------------------------
# GLEW
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED GLEW_DIR AND NOT EXISTS ${GLEW_DIR})
  message(FATAL_ERROR "GLEW_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj GLEW)
set(proj_DEPENDENCIES )
set(GLEW_DEPENDS ${proj})

if(NOT DEFINED GLEW_DIR)

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/glew-cmake-1.10.0.tar.gz
     URL_MD5 1e84666af239ee1ce14e2d0725764cc3
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(GLEW_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
