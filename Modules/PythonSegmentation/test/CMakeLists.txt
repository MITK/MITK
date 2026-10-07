MITK_CREATE_MODULE_TESTS(EXTRA_DEPENDS MitkPython)

if(TARGET ${TESTDRIVER})
  get_filename_component(VOXTELL_BRIDGE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../resource/VoxTell/voxtell_bridge.py" ABSOLUTE)

  target_compile_definitions(${TESTDRIVER} PRIVATE
    MITK_VOXTELL_BRIDGE_FILE="${VOXTELL_BRIDGE_FILE}")

  if(APPLE)
    find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED MODULE)

    foreach(test_cpp IN LISTS MODULE_TESTS)
      get_filename_component(test ${test_cpp} NAME_WE)
      if(TEST ${test})
        set_tests_properties(${test} PROPERTIES LABELS "Python" ENVIRONMENT "DYLD_INSERT_LIBRARIES=${Python3_LIBRARIES}")
      endif()
    endforeach()
  endif()
endif()
