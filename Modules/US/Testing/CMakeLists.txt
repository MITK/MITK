MITK_CREATE_MODULE_TESTS()
if(BUILD_TESTING AND MODULE_IS_ENABLED)
mitkAddCustomModuleTest(mitkUSImageVideoSourceTest mitkUSImageVideoSourceTest
                        ${MITK_DATA_DIR}/CommonTestData/bunny_640x480.avi						
)

endif()