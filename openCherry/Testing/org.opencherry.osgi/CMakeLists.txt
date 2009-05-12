INCLUDE(${OPENCHERRY_BASE_DIR}/CMake/cherryTestingHelpers.cmake)
INCLUDE_DIRECTORIES(${OPENCHERRY_SOURCE_DIR}/org.opencherry.osgi/src)

# tests with no extra command line parameter
SET(OPENCHERRY_TESTS
  cherryMessageTest.cpp
  cherryObjectTest.cpp
  cherrySmartPointerTest.cpp
  cherryWeakPointerTest.cpp
)

OPENCHERRY_CREATE_TESTS()