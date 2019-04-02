#-----------------------------------------------------------------------------
# rasqal
#-----------------------------------------------------------------------------

if(MITK_USE_Rasqal)

# Sanity checks
if(DEFINED Rasqal_DIR AND NOT EXISTS ${Rasqal_DIR})
  message(FATAL_ERROR "Rasqal_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Rasqal)
set(proj_DEPENDENCIES ${Raptor2_DEPENDS} ${PCRE_DEPENDS})
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED Rasqal_DIR)

  set(additional_cmake_args )
  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/rasqal-0.9.32.tar.gz
     URL_MD5 dc7c6107de00c47f85f6ab7db164a136
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Rasqal-0.9.32.patch
     LIST_SEPARATOR ${sep}
     CMAKE_GENERATOR ${gen}
     CMAKE_GENERATOR_PLATFORM ${gen_platform}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       "-DCMAKE_C_FLAGS:STRING=-DPCRE_STATIC ${CMAKE_C_FLAGS}"
       -DRASQAL_REGEX:STRING=pcre
       -DCMAKE_PREFIX_PATH:STRING=${PCRE_DIR}^^${REDLAND_INSTALL_DIR}
       -DPCRE_INCLUDE_DIR:PATH=${PCRE_DIR}/include
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix}/lib/rasqal/cmake)
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
