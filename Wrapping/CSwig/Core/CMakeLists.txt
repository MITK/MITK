# create the mitkCorePython libraries
INCLUDE(wrapSources.cmake)

SET(MASTER_INDEX_FILES "${CMAKE_CURRENT_BINARY_DIR}/Mitk.mdx"
)

SET(mitkCorePythonDependencies Mitk debug vtkCommonPythonD optimized vtkCommonPython)


MITK_WRAP_LIBRARY("${WRAP_SOURCES_TEST}" Mitk Core
  "" "" "${mitkCorePythonDependencies}")

