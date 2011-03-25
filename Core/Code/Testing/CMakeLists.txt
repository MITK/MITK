MITK_CREATE_MODULE_TESTS(LABELS MITK-Core)
# MITK_INSTALL_TARGETS(EXECUTABLES MitkTestDriver)

ADD_TEST(mitkPicFileReaderTest_emptyFile ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkPicFileReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/emptyFile.pic)
SET_PROPERTY(TEST mitkPicFileReaderTest_emptyFile PROPERTY LABELS MITK-Core)

ADD_TEST(mitkPicFileReaderTest_emptyGzipFile ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkPicFileReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/emptyFile.pic.gz)
SET_PROPERTY(TEST mitkPicFileReaderTest_emptyGzipFile PROPERTY LABELS MITK-Core)

ADD_TEST(mitkDICOMLocaleTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkDICOMLocaleTest ${MITK_DATA_DIR}/spacing-ok.dcm)
SET_PROPERTY(TEST mitkDICOMLocaleTest PROPERTY LABELS MITK-Core)

ADD_TEST(mitkEventMapperTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkEventMapperTest ${MITK_DATA_DIR}/TestStateMachine1.xml ${MITK_DATA_DIR}/TestStateMachine2.xml)
SET_PROPERTY(TEST mitkEventMapperTest PROPERTY LABELS MITK-Core)

ADD_TEST(mitkNodeDependentPointSetInteractorTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkNodeDependentPointSetInteractorTest ${MITK_DATA_DIR}/Pic3D.pic.gz ${MITK_DATA_DIR}/BallBinary30x30x30.pic.gz)
SET_PROPERTY(TEST mitkNodeDependentPointSetInteractorTest PROPERTY LABELS MITK-Core)

ADD_TEST(mitkDataStorageTest_US4DCyl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkDataStorageTest ${MITK_DATA_DIR}/US4DCyl.pic.gz)
SET_PROPERTY(TEST mitkDataStorageTest_US4DCyl PROPERTY LABELS MITK-Core)

ADD_TEST(mitkStateMachineFactoryTest_TestStateMachine1_2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkStateMachineFactoryTest ${MITK_DATA_DIR}/TestStateMachine1.xml ${MITK_DATA_DIR}/TestStateMachine2.xml)
SET_PROPERTY(TEST mitkStateMachineFactoryTest_TestStateMachine1_2 PROPERTY LABELS MITK-Core)
