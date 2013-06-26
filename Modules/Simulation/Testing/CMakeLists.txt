if(MITK_USE_SOFA)
  MITK_CREATE_MODULE_TESTS()

  if(WIN32 OR APPLE OR MITK_ENABLE_GUI_TESTING)
    mitkAddCustomModuleTest(mitkSimulationTest mitkSimulationTest ${MITK_DATA_DIR}/Simulation/liver.scn 0.02 4384)
  endif()
endif()
