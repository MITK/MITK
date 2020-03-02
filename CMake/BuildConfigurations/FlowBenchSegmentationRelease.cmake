include(${CMAKE_CURRENT_LIST_DIR}/FlowBenchSegmentation.cmake)

set(MITK_VTK_DEBUG_LEAKS OFF CACHE BOOL "Enable VTK Debug Leaks" FORCE)
set(MITK_BUILD_APP_Workbench OFF CACHE BOOL "Build MITK Workbench" FORCE)
set(MITK_WHITELIST "FlowBenchSegmentation" CACHE STRING "MITK FlowBench Whitelist" FORCE)

find_package(Doxygen REQUIRED)

# Ensure that the in-application help can be build
set(BLUEBERRY_QT_HELP_REQUIRED ON CACHE BOOL "Required Qt help documentation in plug-ins" FORCE)
