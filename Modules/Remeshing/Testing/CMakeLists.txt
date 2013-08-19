if(MITK_USE_ACVD)
  MITK_CREATE_MODULE_TESTS()

  mitkAddCustomModuleTest(mitkACVDTest mitkACVDTest ${MITK_DATA_DIR}/binary.stl 0 1228 1.0 10 0.0 1 0 0)
endif()
