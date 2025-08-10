mitk_create_module_tests()

if(APPLE)
  foreach(test_cpp IN LISTS MODULE_TESTS)
    get_filename_component(test ${test_cpp} NAME_WE)
    set_tests_properties(${test} PROPERTIES ENVIRONMENT "DYLD_INSERT_LIBRARIES=${Python3_LIBRARIES}")
  endforeach()
endif()
