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

    set(additional_cmake_args )
    if(MITK_USE_Python)
      list(APPEND additional_cmake_args
           -DBUILD_NEW_PYTHON_SUPPORT:BOOL=ON
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

    set(opencv_url ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/OpenCV-2.4.2.tar.bz2)
    set(opencv_url_md5 d5d13c4a65dc96cdfaad54767e428215)

    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      URL ${opencv_url}
      URL_MD5 ${opencv_url_md5}
      INSTALL_COMMAND ""
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
