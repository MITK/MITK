mitk_create_module_tests()

if(TARGET ${TESTDRIVER})
  set_property(TEST mitkSplineVtkMapper3DTest PROPERTY RUN_SERIAL TRUE)
endif()
