#-----------------------------------------------------------------------------
# tinyxml2
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED tinyxml2_DIR AND NOT EXISTS ${tinyxml2_DIR})
  message(FATAL_ERROR "tinyxml2_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj tinyxml2)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED tinyxml2_DIR)

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
     GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
     GIT_TAG 8.0.0
     CMAKE_GENERATOR ${gen}
     CMAKE_GENERATOR_PLATFORM ${gen_platform}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
       -DBUILD_TESTING:BOOL=OFF
       -DBUILD_TESTS:BOOL=OFF
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
