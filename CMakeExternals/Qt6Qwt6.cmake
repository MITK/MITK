#-----------------------------------------------------------------------------
# Qwt
#-----------------------------------------------------------------------------

if(MITK_USE_Qt6Qwt6)

# Sanity checks
if(DEFINED Qt6Qwt6_DIR AND NOT EXISTS ${Qt6Qwt6_DIR})
  message(FATAL_ERROR "Qt6Qwt6_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Qt6Qwt6)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED ${proj}_DIR)

  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     GIT_REPOSITORY https://github.com/MITK/Qwt.git
     GIT_TAG 149ab80d5434bf1702c14886b0e353292a9a1d7e # branch: cmake (synced from https://github.com/MehdiChinoune/qwt)
     CMAKE_GENERATOR ${gen}
     CMAKE_GENERATOR_PLATFORM ${gen_platform}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
     CMAKE_CACHE_ARGS
       -DQWT_QT_VERSION_MAJOR:STRING=6
       -DQWT_DESIGNER:BOOL=OFF
       -DQWT_OPENGL:BOOL=OFF
       -DQWT_POLAR:BOOL=OFF
       -DQWT_SVG:BOOL=OFF
       -DQWT_WIDGETS:BOOL=OFF
       ${ep_common_cache_args}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(${proj}_DIR ${ep_prefix}/lib/cmake/${proj})
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()

endif()
