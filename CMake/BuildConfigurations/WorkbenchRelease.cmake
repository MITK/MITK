include(${CMAKE_CURRENT_LIST_DIR}/Default.cmake)

set(MITK_VTK_DEBUG_LEAKS OFF CACHE BOOL "Enable VTK Debug Leaks" FORCE)

# Ensure that the in-application help can be build
set(BLUEBERRY_QT_HELP_REQUIRED ON CACHE BOOL "Required Qt help documentation in plug-ins" FORCE)
