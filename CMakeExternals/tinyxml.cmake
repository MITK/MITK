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

  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/tinyxml_2_6_2.tar.gz
     URL_MD5 c1b864c96804a10526540c664ade67f0
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/tinyxml-2.6.2.patch
       COMMAND ${CMAKE_COMMAND} -Dproj=${proj} -Dproj_target:STRING=tinyxml -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake
     CMAKE_GENERATOR ${gen}
     CMAKE_GENERATOR_PLATFORM ${gen_platform}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
