# create the mitkCorePython libraries
include(Controllers/wrapSources.cmake)
set(WRAP_SOURCES_TEST
  ${WRAP_SOURCES_TEST}
  wrap_mitkCorePython)

set(MASTER_INDEX_FILES "${CMAKE_CURRENT_BINARY_DIR}/Mitk.mdx"
)
foreach(source ${WRAP_SOURCES_TEST})
  message("${source}")
endforeach(source)

MITK_WRAP_LIBRARY("${WRAP_SOURCES_TEST}" Mitk Core
  "" "" Mitk)

