# Create TestDriver and default tests for openCherry
#
# CMake variables used:
#
# ${CHERRY_TESTS} : filenames of all tests that run without a parameter
# ${CHERRY_CUSTOM_TESTS} : filenames of custom tests which are just added to the TestDriver. Execution
#                              of these has to be specified manually with the ADD_TEST CMake command.
# 
MACRO(OPENCHERRY_CREATE_TESTS)
  
  _MACRO_CREATE_PLUGIN_NAME(plugin_name 
                            INPUT ${CMAKE_CURRENT_SOURCE_DIR}
                            BASEDIR ${OPENCHERRY_BASE_DIR}/Testing)
  STRING(REPLACE . _ plugin_target ${plugin_name})
  
  INCLUDE_DIRECTORIES(${OPENCHERRY_BASE_DIR}/Testing)
  INCLUDE_DIRECTORIES(${Poco_INCLUDE_DIRS})
  
  #
  # Create the TestDriver binary which contains all the tests.
  #  
  CREATE_TEST_SOURCELIST(cherry_test_sources TestDriver_${plugin_target}.cpp 
    ${OPENCHERRY_TESTS} ${OPENCHERRY_CUSTOM_TESTS} )
  
  ADD_EXECUTABLE(TestDriver_${plugin_target} ${cherry_test_sources} ${OPENCHERRY_BASE_DIR}/Testing/cherryTestManager.cpp)
  TARGET_LINK_LIBRARIES(TestDriver_${plugin_target} optimized ${plugin_target} debug ${plugin_target}${OPENCHERRY_DEBUG_POSTFIX})
  TARGET_LINK_LIBRARIES(TestDriver_${plugin_target} optimized PocoFoundation debug PocoFoundationd )
  #
  # Now tell CMake which tests should be run. This is done automatically 
  # for all tests in CHERRY_TESTS 
  #
  FOREACH( test ${OPENCHERRY_TESTS} )
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
    ADD_TEST(${TName} ${EXECUTABLE_OUTPUT_PATH}/TestDriver_${plugin_target} ${TName})
  ENDFOREACH( test )

ENDMACRO(OPENCHERRY_CREATE_TESTS)

