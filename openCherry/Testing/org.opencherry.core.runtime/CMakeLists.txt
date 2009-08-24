INCLUDE(${OPENCHERRY_SOURCE_DIR}/CMake/cherryTestingHelpers.cmake)
INCLUDE_DIRECTORIES(${OPENCHERRY_PLUGINS_SOURCE_DIR}/org.opencherry.osgi/src)
INCLUDE_DIRECTORIES(${OPENCHERRY_PLUGINS_SOURCE_DIR}/org.opencherry.core.runtime/src)
INCLUDE_DIRECTORIES(${OPENCHERRY_PLUGINS_SOURCE_DIR}/org.opencherry.core.runtime/src/internal)

# tests with no extra command line parameter
SET(OPENCHERRY_TESTS
  cherryPreferencesServiceTest.cpp
  cherryPreferencesTest.cpp
  cherryXMLPreferencesStorageTest.cpp
)

OPENCHERRY_CREATE_TESTS()