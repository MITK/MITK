#-----------------------------------------------------------------------------
# OpenCV
#-----------------------------------------------------------------------------

if(MITK_USE_OpenCV)

  # Sanity checks
  if(DEFINED OpenCV_DIR AND NOT EXISTS ${OpenCV_DIR})
    message(FATAL_ERROR "OpenCV_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj OpenCV)
  set(proj_DEPENDENCIES)
  set(OpenCV_DEPENDS ${proj})

  if(NOT DEFINED OpenCV_DIR)

    set(additional_cmake_args)

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    mitk_query_custom_ep_vars()

    ExternalProject_Add(${proj}
      GIT_REPOSITORY https://github.com/opencv/opencv.git
      GIT_TAG 4.6.0
      LIST_SEPARATOR ${sep}
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        -DBUILD_JAVA:BOOL=OFF
        -DBUILD_opencv_ts:BOOL=OFF
        -DBUILD_PERF_TESTS:BOOL=OFF
        -DBUILD_opencv_python3:BOOL=OFF
        -DBUILD_opencv_python_bindings_generator:BOOL=OFF
        -DBUILD_opencv_python_tests:BOOL=OFF
        -DWITH_QT:BOOL=OFF
        -DBUILD_TESTS:BOOL=OFF
        -DBUILD_DOCS:BOOL=OFF
        -DBUILD_EXAMPLES:BOOL=OFF
        -DWITH_CUDA:BOOL=OFF
        -DWITH_VTK:BOOL=OFF
        -DENABLE_CXX11:BOOL=ON
        -DWITH_IPP:BOOL=OFF
        -DBUILD_IPP_IW:BOOL=OFF
        -DENABLE_PRECOMPILED_HEADERS:BOOL=OFF
        ${additional_cmake_args}
        ${${proj}_CUSTOM_CMAKE_ARGS}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        ${${proj}_CUSTOM_CMAKE_CACHE_ARGS}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
        ${${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS}
      DEPENDS ${proj_DEPENDENCIES}
      )

    set(OpenCV_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
