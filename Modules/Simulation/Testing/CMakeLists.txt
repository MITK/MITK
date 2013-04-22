MITK_CREATE_MODULE_TESTS()
mitkAddCustomModuleTest(mitkSimulationTest mitkSimulationTest ${MITK_DATA_DIR}/Simulation/liver.scn)
mitkAddCustomModuleTest(mitkSimulationMapper3DTest mitkSimulationMapper3DTest ${MITK_DATA_DIR}/Simulation/liver.scn -V ${MITK_DATA_DIR}/Simulation/mapper1024x768REF.png)
mitkAddCustomModuleTest(mitkSimulationDrawToolTest mitkSimulationDrawToolTest ${MITK_DATA_DIR}/Simulation/liver.scn -V ${MITK_DATA_DIR}/Simulation/drawTool1024x768REF.png)
