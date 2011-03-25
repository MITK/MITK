MITK_CREATE_MODULE_TESTS(LABELS MITK-Core)
# MITK_INSTALL_TARGETS(EXECUTABLES MitkTestDriver)

mitkAddCustomModuleTest(mitkPicFileReaderTest_emptyFile mitkPicFileReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/emptyFile.pic)

mitkAddCustomModuleTest(mitkPicFileReaderTest_emptyGzipFile mitkPicFileReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/emptyFile.pic.gz)

mitkAddCustomModuleTest(mitkDICOMLocaleTest mitkDICOMLocaleTest ${MITK_DATA_DIR}/spacing-ok.dcm)

mitkAddCustomModuleTest(mitkEventMapperTest mitkEventMapperTest ${MITK_DATA_DIR}/TestStateMachine1.xml ${MITK_DATA_DIR}/TestStateMachine2.xml)

mitkAddCustomModuleTest(mitkNodeDependentPointSetInteractorTest mitkNodeDependentPointSetInteractorTest ${MITK_DATA_DIR}/Pic3D.pic.gz ${MITK_DATA_DIR}/BallBinary30x30x30.pic.gz)

mitkAddCustomModuleTest(mitkDataStorageTest_US4DCyl mitkDataStorageTest ${MITK_DATA_DIR}/US4DCyl.pic.gz)

mitkAddCustomModuleTest(mitkStateMachineFactoryTest_TestStateMachine1_2 mitkStateMachineFactoryTest ${MITK_DATA_DIR}/TestStateMachine1.xml ${MITK_DATA_DIR}/TestStateMachine2.xml)

