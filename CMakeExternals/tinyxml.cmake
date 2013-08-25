#-----------------------------------------------------------------------------
# tinyxml
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED tinyxml_DIR AND NOT EXISTS ${tinyxml_DIR})
  message(FATAL_ERROR "tinyxml_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj tinyxml)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED tinyxml_DIR)

  set(additional_cmake_args )
  if(WIN32)
    set(additional_cmake_args -DBUILD_SHARED_LIBS:BOOL=OFF)
  endif()

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/tinyxml-cmake-2.6.2.tar.gz
     URL_MD5 1ef1c3cf1f070ce57ab60a406a26a5c5
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
