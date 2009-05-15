FIND_PACKAGE(openCherry)

SET(TEST_BUNDLES
 org.opencherry.osgi
)

FOREACH(testbundle ${TEST_BUNDLES})
  ADD_SUBDIRECTORY(${testbundle})
ENDFOREACH(testbundle ${TEST_BUNDLES})