# create the mitkCorePython libraries
include(wrapSources.cmake)

set(MASTER_INDEX_FILES "${CMAKE_CURRENT_BINARY_DIR}/Mitk.mdx"
)

set(mitkCorePythonDependencies Mitk debug vtkCommonPythonD optimized vtkCommonPython)


MITK_WRAP_LIBRARY("${WRAP_SOURCES_TEST}" Mitk Core
  "" "" "${mitkCorePythonDependencies}")

