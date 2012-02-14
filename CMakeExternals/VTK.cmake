#-----------------------------------------------------------------------------
# VTK
#-----------------------------------------------------------------------------

IF(WIN32)
  OPTION(VTK_USE_SYSTEM_FREETYPE OFF)
ELSE(WIN32)
  OPTION(VTK_USE_SYSTEM_FREETYPE ON)
ENDIF(WIN32)

# Sanity checks
IF(DEFINED VTK_DIR AND NOT EXISTS ${VTK_DIR})
  MESSAGE(FATAL_ERROR "VTK_DIR variable is defined but corresponds to non-existing directory")
ENDIF()

SET(proj VTK)
SET(proj_DEPENDENCIES )
SET(VTK_DEPENDS ${proj})

IF(NOT DEFINED VTK_DIR)

  SET(additional_cmake_args )
  IF(MINGW)
    SET(additional_cmake_args
        -DCMAKE_USE_WIN32_THREADS:BOOL=ON
        -DCMAKE_USE_PTHREADS:BOOL=OFF
        -DVTK_USE_VIDEO4WINDOWS:BOOL=OFF # no header files provided by MinGW
        )
  ENDIF()

  IF(MITK_USE_Python)
    LIST(APPEND additional_cmake_args
         -DVTK_WRAP_PYTHON:BOOL=ON
         -DVTK_USE_TK:BOOL=OFF
         -DVTK_WINDOWS_PYTHON_DEBUGGABLE:BOOL=OFF
        )
  ENDIF()
  
  IF(MITK_USE_QT)
    LIST(APPEND additional_cmake_args
        -DDESIRED_QT_VERSION:STRING=4
        -DVTK_USE_GUISUPPORT:BOOL=ON
        -DVTK_USE_QVTK_QTOPENGL:BOOL=ON
        -DVTK_USE_QT:BOOL=ON
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
     )
  ENDIF()

  OPTION(MITK_USE_VTK_5_8_IN_SUPERBUILD OFF)
  IF(MITK_USE_VTK_5_8_IN_SUPERBUILD)
    SET(VTK_URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/vtk-5.8.0.tar.gz)
    SET(VTK_URL_MD5 37b7297d02d647cc6ca95b38174cb41f)
  ELSE()
    SET(VTK_URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/vtk-5.6.1.tar.gz)
    SET(VTK_URL_MD5 b80a76435207c5d0f74dfcab15b75181)
  ENDIF()

  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
    BINARY_DIR ${proj}-src
    PREFIX ${proj}-cmake
    URL ${VTK_URL}
    URL_MD5 ${VTK_URL_MD5}
    INSTALL_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
        ${ep_common_args}
        -DVTK_WRAP_TCL:BOOL=OFF
        -DVTK_WRAP_PYTHON:BOOL=OFF
        -DVTK_WRAP_JAVA:BOOL=OFF
        -DBUILD_SHARED_LIBS:BOOL=ON 
        -DVTK_USE_PARALLEL:BOOL=ON
        -DVTK_USE_CHARTS:BOOL=OFF
        -DVTK_USE_QTCHARTS:BOOL=ON
        -DVTK_USE_GEOVIS:BOOL=OFF
        -DVTK_USE_SYSTEM_FREETYPE:BOOL=${VTK_USE_SYSTEM_FREETYPE}
        -DVTK_USE_QVTK_QTOPENGL:BOOL=OFF
        ${additional_cmake_args}
     DEPENDS ${proj_DEPENDENCIES}
    )
  SET(VTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

ELSE()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  
ENDIF()
