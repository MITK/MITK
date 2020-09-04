#Configuration for a minimal setup to build all cmd apps provided by the core and classification module
include(${CMAKE_CURRENT_LIST_DIR}/CoreCmdApps.cmake)

set(MITK_WHITELIST "ClassificationCmdApps" CACHE STRING "" FORCE)
set(MITK_USE_Vigra ON CACHE BOOL "" FORCE)

#Following packages are needed due to the current dependency
#of some classification cmd apps to QtWidgetExt (see also
#disucssion in T27702)
set(MITK_USE_Qt5 ON CACHE BOOL "" FORCE)
set(MITK_USE_CTK ON CACHE BOOL "" FORCE)
set(MITK_USE_Qwt ON CACHE BOOL "" FORCE)

#Following packages are needed due to the current dependency
#of some classification cmd apps to QtWidgetExt (see also
#disucssion in T27702)
set(MITK_CONFIG_PACKAGES
  Qt5
  CTK
  Qwt
  Vigra
)

if(NOT MITK_USE_SUPERBUILD)
  set(BUILD_ClassificationMiniApps ON CACHE BOOL "" FORCE)
endif()
