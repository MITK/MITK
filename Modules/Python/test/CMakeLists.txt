mitk_create_module_tests()

if(TARGET ${TESTDRIVER})
  target_compile_definitions(${TESTDRIVER} PRIVATE
    MITK_PYTHON_PYTEST_RUNNER="${CMAKE_CURRENT_SOURCE_DIR}/pytest/run_pytest_suite.py")

  if(APPLE)
    foreach(test_cpp IN LISTS MODULE_TESTS)
      get_filename_component(test ${test_cpp} NAME_WE)
      if(TEST ${test})
        set_tests_properties(${test} PROPERTIES LABELS "Python" ENVIRONMENT "DYLD_INSERT_LIBRARIES=${Python3_LIBRARIES}")
      endif()
    endforeach()
  endif()
endif()
