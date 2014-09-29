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
    )

    if(MITK_USE_Python)
      #message(STATUS "PYTHON_EXECUTABLE: ${PYTHON_EXECUTABLE}")
      #message(STATUS "PYTHON_DEBUG_LIBRARY: ${PYTHON_DEBUG_LIBRARY}")
      #message(STATUS "PYTHON_INCLUDE_DIR: ${PYTHON_INCLUDE_DIR}")
      #message(STATUS "PYTHON_LIBRARY: ${PYTHON_LIBRARY}")
      list(APPEND additional_cmake_args
         -DBUILD_opencv_python:BOOL=ON
         -DBUILD_NEW_PYTHON_SUPPORT:BOOL=ON
         -DPYTHON_DEBUG_LIBRARY:FILEPATH=${PYTHON_DEBUG_LIBRARY}
         -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
         -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
         -DPYTHON_INCLUDE_DIR2:PATH=${PYTHON_INCLUDE_DIR2}
         -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}

         #-DPYTHON_LIBRARIES=${PYTHON_LIBRARY}
         #-DPYTHON_DEBUG_LIBRARIES=${PYTHON_DEBUG_LIBRARIES}
          )
      if(NOT MITK_USE_SYSTEM_PYTHON)
        list(APPEND proj_DEPENDENCIES Python Numpy)
        # export python home
        set(ENV{PYTHONHOME} "${Python_DIR}")
      endif()
    else()
      list(APPEND additional_cmake_args
         -DBUILD_opencv_python:BOOL=OFF
         -DBUILD_NEW_PYTHON_SUPPORT:BOOL=OFF
          )
    endif()

    # 12-05-02, muellerm, added QT usage by OpenCV if QT is used in MITK
    # 12-09-11, muellerm, removed automatic usage again, since this will struggle with the MITK Qt application object
    if(MITK_USE_QT)
      list(APPEND additional_cmake_args
           -DWITH_QT:BOOL=OFF
           -DWITH_QT_OPENGL:BOOL=OFF
           -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
          )
    endif()

    set(OpenCV_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchOpenCV-2.4.8.2.cmake)


    set(opencv_url ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/OpenCV-2.4.8.2.tar.gz)
    set(opencv_url_md5 07fa7c1d225ea7fe8eeb1270a6b00e69)

    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      URL ${opencv_url}
      URL_MD5 ${opencv_url_md5}
      INSTALL_COMMAND ""
      PATCH_COMMAND ${OpenCV_PATCH_COMMAND}
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        -DBUILD_DOCS:BOOL=OFF
        -DBUILD_TESTS:BOOL=OFF
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_DOXYGEN_DOCS:BOOL=OFF
        -DWITH_CUDA:BOOL=ON
        ${additional_cmake_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(OpenCV_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
