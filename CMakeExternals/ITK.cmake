#-----------------------------------------------------------------------------
# ITK
#-----------------------------------------------------------------------------

# Sanity checks
IF(DEFINED ITK_DIR AND NOT EXISTS ${ITK_DIR})
  MESSAGE(FATAL_ERROR "ITK_DIR variable is defined but corresponds to non-existing directory")
ENDIF()

SET(proj ITK)
SET(proj_DEPENDENCIES GDCM)
SET(ITK_DEPENDS ${proj})

IF(NOT DEFINED ITK_DIR)

  SET(additional_cmake_args )
  IF(MINGW)
    SET(additional_cmake_args
        -DCMAKE_USE_WIN32_THREADS:BOOL=ON
        -DCMAKE_USE_PTHREADS:BOOL=OFF)
  ENDIF()

  ExternalProject_Add(${proj}
     URL http://mitk.org/download/thirdparty/InsightToolkit-3.20.0.tar.gz
     BINARY_DIR ${proj}-build
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       -DBUILD_TESTING:BOOL=OFF
       -DBUILD_EXAMPLES:BOOL=OFF
       -DITK_USE_SYSTEM_GDCM:BOOL=ON
       -DGDCM_DIR:PATH=${GDCM_DIR}
     DEPENDS ${proj_DEPENDENCIES}
    )

  SET(ITK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

ELSE()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

ENDIF()
