MITK_CREATE_MODULE_TESTS()
mitkAddCustomModuleTest(mitkSimulationTest mitkSimulationTest ${MITK_DATA_DIR}/Simulation/visualModel.scn)
mitkAddCustomModuleTest(mitkSimulationMapper3DTest mitkSimulationMapper3DTest ${MITK_DATA_DIR}/Simulation/visualModel.scn -V ${MITK_DATA_DIR}/Simulation/visualModel640x480REF.png)
