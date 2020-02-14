if(UNIX AND NOT APPLE)
  # See T26955.
  set(MODULE_CUSTOM_TESTS
    mitkPythonTest.cpp
  )
else()
  set(MODULE_TESTS
    mitkPythonTest.cpp
  )
endif()
