INCLUDE("${OPENCHERRY_BINARY_DIR}/openCherryConfig.cmake")

SET(TEST_BUNDLES
 org.opencherry.osgi
 org.opencherry.core.runtime
)

FOREACH(testbundle ${TEST_BUNDLES})
  ADD_SUBDIRECTORY(${testbundle})
ENDFOREACH(testbundle ${TEST_BUNDLES})