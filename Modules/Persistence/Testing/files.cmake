set(MODULE_TESTS
mitkPersistenceTest.cpp
)
# Create an artificial module initializing class for
# the usServiceListenerTest.cpp
usFunctionGenerateExecutableInit(TEST_CPP_FILES
                                 IDENTIFIER ${MODULE_NAME}TestDriver
                                )