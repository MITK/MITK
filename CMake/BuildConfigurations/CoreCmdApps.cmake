include(${CMAKE_CURRENT_LIST_DIR}/Minimal.cmake)

if(NOT MITK_USE_SUPERBUILD)
  set(BUILD_CoreCmdApps ON CACHE BOOL "" FORCE)
endif()
