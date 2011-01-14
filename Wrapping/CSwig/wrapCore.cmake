# create the mitkCorePython libraries
INCLUDE(Controllers/wrapSources.cmake)
SET(WRAP_SOURCES_TEST
	${WRAP_SOURCES_TEST}
	wrap_mitkCorePython)

SET(MASTER_INDEX_FILES "${CMAKE_CURRENT_BINARY_DIR}/mitkCore.mdx"
)
FOREACH(source ${WRAP_SOURCES_TEST})
	message("${source}")
ENDFOREACH(source)

MITK_WRAP_LIBRARY("${WRAP_SOURCES_TEST}" mitkCore Core
  "" "" mitkCore)

