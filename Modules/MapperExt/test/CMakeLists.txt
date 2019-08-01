mitk_create_module_tests()

if(TARGET ${TESTDRIVER} AND MITK_ENABLE_RENDERING_TESTING)
  set_property(TEST mitkSplineVtkMapper3DTest PROPERTY RUN_SERIAL TRUE)
endif()
