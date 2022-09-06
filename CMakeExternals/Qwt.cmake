#-----------------------------------------------------------------------------
# Qwt
#-----------------------------------------------------------------------------

if(MITK_USE_Qwt)

# Sanity checks
if(DEFINED Qwt_DIR AND NOT EXISTS ${Qwt_DIR})
  message(FATAL_ERROR "Qwt_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Qwt)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED ${proj}_DIR)

  set(additional_cmake_args
  "-DQt5Svg_DIR:PATH=${Qt5Svg_DIR}"
  "-DQt5OpenGL_DIR:PATH=${Qt5OpenGL_DIR}"
  "-DQt5PrintSupport_DIR:PATH=${Qt5PrintSupport_DIR}"
  "-DQt5Concurrent_DIR:PATH=${Qt5Concurrent_DIR}"
  "-DQt5Designer_DIR:PATH=${Qt5_DIR}Designer"
  )
  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     GIT_REPOSITORY https://github.com/MITK/Qwt.git
     GIT_TAG v6.2.0-patched
     CMAKE_GENERATOR ${gen}
     CMAKE_GENERATOR_PLATFORM ${gen_platform}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
     CMAKE_CACHE_ARGS
       -DQWT_BUILD_DESIGNER_PLUGIN:BOOL=OFF
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

endif()
