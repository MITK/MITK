MITK_CREATE_MODULE_TESTS(LABELS MITK-Core)
# MITK_INSTALL_TARGETS(EXECUTABLES MitkTestDriver)

mitkAddCustomModuleTest(mitkDICOMLocaleTest_spacingOk_CT mitkDICOMLocaleTest ${MITK_DATA_DIR}/spacing-ok-ct.dcm)
mitkAddCustomModuleTest(mitkDICOMLocaleTest_spacingOk_MR mitkDICOMLocaleTest ${MITK_DATA_DIR}/spacing-ok-mr.dcm)
mitkAddCustomModuleTest(mitkDICOMLocaleTest_spacingOk_SC mitkDICOMLocaleTest ${MITK_DATA_DIR}/spacing-ok-sc.dcm)

mitkAddCustomModuleTest(mitkEventMapperTest_Test1And2 mitkEventMapperTest ${MITK_DATA_DIR}/TestStateMachine1.xml ${MITK_DATA_DIR}/TestStateMachine2.xml)

#mitkAddCustomModuleTest(mitkNodeDependentPointSetInteractorTest mitkNodeDependentPointSetInteractorTest ${MITK_DATA_DIR}/Pic3D.pic.gz ${MITK_DATA_DIR}/BallBinary30x30x30.pic.gz)
mitkAddCustomModuleTest(mitkNodeDependentPointSetInteractorTest mitkNodeDependentPointSetInteractorTest  ${MITK_DATA_DIR}/Pic3D.nrrd ${MITK_DATA_DIR}/BallBinary30x30x30.nrrd)

mitkAddCustomModuleTest(mitkDataStorageTest_US4DCyl mitkDataStorageTest ${MITK_DATA_DIR}/US4DCyl.nrrd)

mitkAddCustomModuleTest(mitkStateMachineFactoryTest_TestStateMachine1_2 mitkStateMachineFactoryTest ${MITK_DATA_DIR}/TestStateMachine1.xml ${MITK_DATA_DIR}/TestStateMachine2.xml)

mitkAddCustomModuleTest(mitkDicomSeriesReaderTest_CTImage mitkDicomSeriesReaderTest ${MITK_DATA_DIR}/TinyCTAbdomen)

mitkAddCustomModuleTest(mitkPointSetReaderTest mitkPointSetReaderTest ${MITK_DATA_DIR}/PointSetReaderTestData.mps)

IF(MITK_ENABLE_GUI_TESTING)
mitkAddCustomModuleTest(mitkImageVtkMapper2D_rgbaImage640x480 mitkImageVtkMapper2DTest
                        ${MITK_DATA_DIR}/RenderingTestData/rgbaImage.png #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/rgbaImage640x480REF.png #corresponding reference screenshot
)
ENDIF()

# see bug 9882
IF(NOT APPLE)
ADD_TEST(mitkPointSetLocaleTest  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkPointSetLocaleTest ${MITK_DATA_DIR}/pointSet.mps)
SET_PROPERTY(TEST mitkPointSetLocaleTest PROPERTY LABELS MITK-Core)
ENDIF()

ADD_TEST(mitkImageTest_brainImage ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkImageTest ${MITK_DATA_DIR}/brain.mhd)
SET_PROPERTY(TEST mitkImageTest_brainImage PROPERTY LABELS MITK-Core)
# ADD_TEST(mitkImageTest_4DImageData ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkImageTest ${MITK_DATA_DIR}/US4DCyl.pic.gz)
# SET_PROPERTY(TEST mitkImageTest_4DImageData PROPERTY LABELS MITK-Core)

ADD_TEST(mitkImageWriterTest_nrrdImage ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkImageWriterTest ${MITK_DATA_DIR}/NrrdWritingTestImage.jpg)
SET_PROPERTY(TEST mitkImageWriterTest_nrrdImage PROPERTY LABELS MITK-Core)

add_subdirectory(DICOMTesting)

add_subdirectory(TestModules)
