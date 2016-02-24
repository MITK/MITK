MITK_CREATE_MODULE_TESTS()

if(MITK_ENABLE_RENDERING_TESTING) ### since the rendering test's do not run in ubuntu, yet, we build them only for other systems or if the user explicitly sets the variable
  SET_PROPERTY(TEST
    mitkSplineVtkMapper3DTest
  PROPERTY RUN_SERIAL TRUE)

endif()
