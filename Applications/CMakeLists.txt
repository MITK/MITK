set(MITK_CPACK_PACKAGE_EXECUTABLES "" CACHE INTERNAL "Collecting windows shortcuts to executables" FORCE)
set(MITK_DIR ${PROJECT_BINARY_DIR})
set(MITK_EXPORTS_FILE_INCLUDED 1)

if(MITK_USE_QT AND QT4_FOUND)

  if(MITK_USE_CTK)
    add_subdirectory(PluginGenerator)
  endif()
  
  if(MITK_BUILD_APP_CoreApp OR MITK_BUILD_ALL_APPS)
    add_subdirectory(CoreApp)
  endif()

  if(MITK_BUILD_APP_ExtApp OR MITK_BUILD_ALL_APPS)
    add_subdirectory(ExtApp)
  endif()

  if(MITK_BUILD_TUTORIAL)
    add_subdirectory(Tutorial)
  endif()
  
  if(MITK_BUILD_APP_mitkDiffusion)
    add_subdirectory(mitkDiffusion)
  endif()

endif()
