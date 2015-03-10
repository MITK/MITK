MITK_CREATE_MODULE_TESTS(PACKAGE_DEPENDS Poco)
if(BUILD_TESTING AND MODULE_IS_ENABLED)
mitkAddCustomModuleTest(mitkUSImageVideoSourceTest mitkUSImageVideoSourceTest
                        ${MITK_DATA_DIR}/CommonTestData/bunny_320x240.avi
)

endif()
