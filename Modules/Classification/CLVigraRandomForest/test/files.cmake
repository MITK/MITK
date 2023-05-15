set(MODULE_TESTS "")

if(NOT APPLE)
  list(APPEND MODULE_TESTS mitkVigraRandomForestTest.cpp)
endif()
