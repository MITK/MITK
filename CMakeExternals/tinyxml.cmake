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
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/tinyxml_2_6_2.tar.gz
     URL_MD5 c1b864c96804a10526540c664ade67f0
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/tinyxml-2.6.2.patch
       COMMAND ${CMAKE_COMMAND} -Dproj=${proj} -Dproj_target:STRING=tinyxml -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake
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
