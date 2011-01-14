# create the mitkCorePython libraries
INCLUDE(wrapSources.cmake)

SET(MASTER_INDEX_FILES "${CMAKE_CURRENT_BINARY_DIR}/mitkCore.mdx"
)

SET(mitkCorePythonDependencies mitkCore debug vtkCommonPythonD optimized vtkCommonPython)


MITK_WRAP_LIBRARY("${WRAP_SOURCES_TEST}" mitkCore Core
  "" "" "${mitkCorePythonDependencies}")

