include(${CMAKE_CURRENT_LIST_DIR}/mitkBuildConfigurationDefault.cmake)

set(MITK_VTK_DEBUG_LEAKS OFF CACHE BOOL "Enable VTK Debug Leaks" FORCE)

# Ensure that the in-application help can be build
if(NOT DOXYGEN_EXECUTABLE)
  message(FATAL_ERROR "DOXYGEN_EXECUTABLE is not set")
endif()
if(NOT BLUEBERRY_USE_QT_HELP)
  message(FATAL_ERROR "BLUEBERRY_USE_QT_HELP is required to be set to ON")
endif()
