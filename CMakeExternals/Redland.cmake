#-----------------------------------------------------------------------------
# redland
#-----------------------------------------------------------------------------

if(MITK_USE_Redland)

# Sanity checks
if(DEFINED Redland_DIR AND NOT EXISTS ${Redland_DIR})
  message(FATAL_ERROR "Redland_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Redland)
set(proj_DEPENDENCIES ${Raptor2_DEPENDS} ${Rasqal_DEPENDS})
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED Redland_DIR)

  set(additional_cmake_args )
  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/redland-1.0.17.tar.gz
     URL_MD5 e5be03eda13ef68aabab6e42aa67715e
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Redland-1.0.17.patch
     CMAKE_GENERATOR ${gen}
     CMAKE_GENERATOR_PLATFORM ${gen_platform}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       -DWITH_THREADS:BOOL=OFF
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix}/lib/redland/cmake)
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
