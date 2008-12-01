INCLUDE(${OPENCHERRY_BASE_DIR}/CMake/TestingHelpers.cmake)
INCLUDE_DIRECTORIES(${OPENCHERRY_SOURCE_DIR}/org.opencherry.osgi)

# tests with no extra command line parameter
SET(OPENCHERRY_TESTS
  cherryMessageTest.cpp
  cherryObjectTest.cpp
  cherrySmartPointerTest.cpp
)

OPENCHERRY_CREATE_TESTS()