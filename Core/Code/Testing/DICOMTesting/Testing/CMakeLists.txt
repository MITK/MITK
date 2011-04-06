
MITK_CREATE_MODULE_TESTS(LABELS MITK-Core)

# verify minimum expectations:
#   files are recognized as DICOM
#   loading files results in a given number of images
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_NoFiles mitkDICOMTestingSanityTest 0)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_CTImage mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-ct.dcm)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_MRImage mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-mr.dcm)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_SCImage mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-sc.dcm)

