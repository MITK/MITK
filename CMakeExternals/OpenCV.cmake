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

    set(additional_cmake_args
      -DBUILD_opencv_java:BOOL=OFF
      -DBUILD_opencv_ts:BOOL=OFF
      -DBUILD_PERF_TESTS:BOOL=OFF
      -DBUILD_opencv_python:BOOL=OFF
      -DBUILD_opencv_python3:BOOL=OFF
      -DBUILD_opencv_python_bindings_generator:BOOL=OFF
      #-DBUILD_NEW_PYTHON_SUPPORT:BOOL=OFF
      )

    # 12-05-02, muellerm, added QT usage by OpenCV if QT is used in MITK
    # 12-09-11, muellerm, removed automatic usage again, since this will struggle with the MITK Qt application object
    if(MITK_USE_Qt5)
      list(APPEND additional_cmake_args
        -DWITH_QT:BOOL=OFF
        -DWITH_QT_OPENGL:BOOL=OFF
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        )
    endif()

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
        )
    endif()

    set(opencv_url ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/opencv-3.4.2.tar.gz)
    set(opencv_url_md5 8aba51c788cac3583bb39a0c24a5888f)
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${opencv_url}
      URL_MD5 ${opencv_url_md5}
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
      ${ep_common_args}
      -DBUILD_TESTS:BOOL=OFF
      -DBUILD_DOCS:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_DOXYGEN_DOCS:BOOL=OFF
      -DWITH_CUDA:BOOL=OFF
      -DWITH_VTK:BOOL=OFF
      -DENABLE_CXX11:BOOL=ON
      -DWITH_IPP:BOOL=OFF
      -DBUILD_IPP_IW:BOOL=OFF
      ${additional_cmake_args}
      CMAKE_CACHE_ARGS
      ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
      ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
      )

    set(OpenCV_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
