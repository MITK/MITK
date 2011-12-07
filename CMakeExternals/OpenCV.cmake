#-----------------------------------------------------------------------------
# OpenCV
#-----------------------------------------------------------------------------

IF(MITK_USE_OpenCV)

  # Sanity checks
  IF(DEFINED OpenCV_DIR AND NOT EXISTS ${OpenCV_DIR})
    MESSAGE(FATAL_ERROR "OpenCV_DIR variable is defined but corresponds to non-existing directory")
  ENDIF()

  SET(proj OpenCV)
  SET(proj_DEPENDENCIES)
  SET(OpenCV_DEPENDS ${proj})

  IF(NOT DEFINED OpenCV_DIR)

  SET(additional_cmake_args )
  IF(MITK_USE_Python)
    LIST(APPEND additional_cmake_args
         -DBUILD_NEW_PYTHON_SUPPORT:BOOL=ON
        )
  ENDIF()

    SET(opencv_url http://mitk.org/download/thirdparty/OpenCV-2.3.0.tar.bz2)

    ExternalProject_Add(${proj}
URL ${opencv_url}
BINARY_DIR ${proj}-build
INSTALL_COMMAND ""
CMAKE_GENERATOR ${gen}
CMAKE_ARGS
  ${ep_common_args}
  -DBUILD_DOCS:BOOL=OFF
  -DBUILD_TESTS:BOOL=OFF
  -DBUILD_EXAMPLES:BOOL=OFF
  -DBUILD_DOXYGEN_DOCS:BOOL=OFF
  ${additional_cmake_args}
DEPENDS ${proj_DEPENDENCIES}
)

    SET(OpenCV_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

  ELSE()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  ENDIF()

ENDIF()
