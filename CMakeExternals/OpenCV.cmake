#-----------------------------------------------------------------------------
# OpenCV
#-----------------------------------------------------------------------------

IF(MITK_USE_OpenCV)

  if (UNIX AND NOT APPLE)

    include(${MITK_SOURCE_DIR}/CMake/mitkMacroGetLinuxDistribution.cmake)
    GetLinuxDistribution()

    if("${LINUX_DISTRIBUTION}" MATCHES "^[Uu][Bb][Uu][Nn][Tt][Uu].*" AND "${LINUX_RELEASE}" MATCHES "11.04")
      set(OpenCV_DIR "/usr/share/opencv/")
    endif()

  endif()

  # Sanity checks
  IF(DEFINED OpenCV_DIR AND NOT EXISTS ${OpenCV_DIR})
    MESSAGE(FATAL_ERROR "OpenCV_DIR variable is defined but corresponds to non-existing directory")
  ENDIF()

  SET(proj OpenCV)
  SET(proj_DEPENDENCIES)
  SET(OpenCV_DEPENDS ${proj})

  IF(NOT DEFINED OpenCV_DIR)

    OPTION(OpenCV_BUILD_NEW_PYTHON_SUPPORT "Build OpenCV Python wrappers" OFF)
    MARK_AS_ADVANCED(OpenCV_BUILD_NEW_PYTHON_SUPPORT)

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
  -DBUILD_NEW_PYTHON_SUPPORT:BOOL=${OpenCV_BUILD_NEW_PYTHON_SUPPORT}
DEPENDS ${proj_DEPENDENCIES}
)

    SET(OpenCV_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

  ELSE()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  ENDIF()

ENDIF()
