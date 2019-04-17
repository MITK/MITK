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

if(MITK_USE_HDF5)
  list(APPEND proj_DEPENDENCIES HDF5)
endif()

if(NOT DEFINED VTK_DIR)

  set(additional_cmake_args )

  # Optionally enable memory leak checks for any objects derived from vtkObject. This
  # will force unit tests to fail if they have any of these memory leaks.
  option(MITK_VTK_DEBUG_LEAKS OFF)
  mark_as_advanced(MITK_VTK_DEBUG_LEAKS)
  list(APPEND additional_cmake_args
    -DVTK_DEBUG_LEAKS:BOOL=${MITK_VTK_DEBUG_LEAKS}
    -DVTK_WRAP_PYTHON:BOOL=OFF
    -DVTK_WINDOWS_PYTHON_DEBUGGABLE:BOOL=OFF
    )

  if(MITK_USE_Qt5)
    list(APPEND additional_cmake_args
      -DVTK_Group_Qt:BOOL=ON
      -DQt5_DIR:PATH=${Qt5_DIR}
      )
  endif()

  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
  endif()

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/VTK-8.1.0.tar.gz
    URL_MD5 4fa5eadbc8723ba0b8d203f05376d932
    CMAKE_GENERATOR ${gen}
    CMAKE_GENERATOR_PLATFORM ${gen_platform}
    CMAKE_ARGS
    ${ep_common_args}
    -DVTK_WRAP_TCL:BOOL=OFF
    -DVTK_WRAP_PYTHON:BOOL=OFF
    -DVTK_WRAP_JAVA:BOOL=OFF
    -DVTK_USE_SYSTEM_FREETYPE:BOOL=${VTK_USE_SYSTEM_FREETYPE}
    -DVTK_LEGACY_REMOVE:BOOL=ON
    -DModule_vtkTestingRendering:BOOL=ON
    ${additional_cmake_args}
    CMAKE_CACHE_ARGS
    ${ep_common_cache_args}
    CMAKE_CACHE_DEFAULT_ARGS
    ${ep_common_cache_default_args}
    DEPENDS ${proj_DEPENDENCIES}
    )

  set(VTK_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
