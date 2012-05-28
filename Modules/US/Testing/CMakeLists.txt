MITK_CREATE_MODULE_TESTS()
if(BUILD_TESTING AND MODULE_IS_ENABLED)
mitkAddCustomModuleTest(mitkUSImageVideoSourceTest mitkUSImageVideoSourceTest
                        ${MITK_DATA_DIR}/CommonTestData/bunny_320x240.avi
)
mitkAddCustomModuleTest(mitkUSPipelineTest mitkUSPipelineTest
                        ${MITK_DATA_DIR}/CommonTestData/bunny_320x240.avi
)

endif()