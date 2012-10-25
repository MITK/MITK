#-----------------------------------------------------------------------------
# VTK
#-----------------------------------------------------------------------------

if(WIN32)
  option(VTK_USE_SYSTEM_FREETYPE OFF)
else(WIN32)
  option(VTK_USE_SYSTEM_FREETYPE ON)
endif(WIN32)

# Sanity checks
if(DEFINED VTK_DIR AND NOT EXISTS ${VTK_DIR})
  message(FATAL_ERROR "VTK_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj VTK)
set(proj_DEPENDENCIES )
set(VTK_DEPENDS ${proj})

if(NOT DEFINED VTK_DIR)

  set(additional_cmake_args )
  if(MINGW)
    set(additional_cmake_args
        -DCMAKE_USE_WIN32_THREADS:BOOL=ON
        -DCMAKE_USE_PTHREADS:BOOL=OFF
        -DVTK_USE_VIDEO4WINDOWS:BOOL=OFF # no header files provided by MinGW
        )
  endif()

  if(MITK_USE_Python)
    list(APPEND additional_cmake_args
         -DVTK_WRAP_PYTHON:BOOL=ON
         -DVTK_USE_TK:BOOL=OFF
         -DVTK_WINDOWS_PYTHON_DEBUGGABLE:BOOL=OFF
        )
  endif()
  
  if(MITK_USE_QT)
    list(APPEND additional_cmake_args
        -DDESIRED_QT_VERSION:STRING=4
        -DVTK_USE_GUISUPPORT:BOOL=ON
        -DVTK_USE_QVTK_QTOPENGL:BOOL=OFF
        -DVTK_USE_QT:BOOL=ON
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
     )
  endif()

  set(VTK_URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/vtk-5.10.0.tar.gz)
  set(VTK_URL_MD5 a0363f78910f466ba8f1bd5ab5437cb9)

  if(APPLE)
    set(VTK_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchVTK-5.10-Mac.cmake)

    ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
    BINARY_DIR ${proj}-build
    PREFIX ${proj}-cmake
    URL ${VTK_URL}
    URL_MD5 ${VTK_URL_MD5}
    INSTALL_COMMAND ""
    PATCH_COMMAND ${VTK_PATCH_COMMAND}
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
        -DVTK_LEGACY_REMOVE:BOOL=ON
        ${additional_cmake_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  else()

    ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
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
         -DVTK_LEGACY_REMOVE:BOOL=ON
         ${additional_cmake_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

  endif()

  set(VTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  
endif()
