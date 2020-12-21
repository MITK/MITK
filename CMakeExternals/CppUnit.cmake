#-----------------------------------------------------------------------------
# CppUnit
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED CppUnit_DIR AND NOT EXISTS ${CppUnit_DIR})
  message(FATAL_ERROR "CppUnit_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj CppUnit)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED CppUnit_DIR)

  set(additional_args )
  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/cppunit-1.15.1.tar.gz
     URL_MD5 9dc669e6145cadd9674873e24943e6dd
     PATCH_COMMAND
       COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/${proj}config.h.cmake <SOURCE_DIR>/config/config.h.cmake
       COMMAND ${CMAKE_COMMAND} -Dproj=${proj} -Dproj_target:STRING=cppunit -P ${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake
     CMAKE_GENERATOR ${gen}
     CMAKE_GENERATOR_PLATFORM ${gen_platform}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_args}
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix}/lib/cmake/CppUnit)
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
