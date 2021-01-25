include(${CMAKE_CURRENT_LIST_DIR}/WorkbenchRelease.cmake)

list(APPEND MITK_CONFIG_PACKAGES
  Vigra
)

list(APPEND MITK_CONFIG_PLUGINS
  org.mitk.gui.qt.basicimageprocessing
  org.mitk.gui.qt.radiomics
)

if(NOT MITK_USE_SUPERBUILD)
  set(BUILD_BasicImageProcessingMiniApps ON CACHE BOOL "" FORCE)
  set(BUILD_ClassificationMiniApps ON CACHE BOOL "" FORCE)
endif()
