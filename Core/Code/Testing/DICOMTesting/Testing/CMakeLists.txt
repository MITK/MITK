
MITK_CREATE_MODULE_TESTS(LABELS MITK-Core)

include(mitkFunctionAddTestLabel)

# verify minimum expectations:
#   files are recognized as DICOM
#   loading files results in a given number of images
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_NoFiles mitkDICOMTestingSanityTest 0)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_CTImage mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-ct.dcm)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_MRImage mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-mr.dcm)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_SCImage mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-sc.dcm)

set(VERIFY_DUMP_CMD VerifyDICOMMitkImageDump)

set(CT_ABDOMEN_DIR ${MITK_DATA_DIR}/TinyCTAbdomen)

# these variables could be passed as parameters to a generic test creation function
set(CURRENT_DATASET_DIR ${CT_ABDOMEN_DIR})
set(TESTS_DIR Tests)
set(INPUT_LISTNAME input)
set(EXPECTED_DUMP expected.dump)

file(GLOB_RECURSE allInputs ${CURRENT_DATASET_DIR}/${TESTS_DIR}/*/${INPUT_LISTNAME})

foreach(input ${allInputs})
  string(REGEX REPLACE ".*${TESTS_DIR}/([^/]+)/.*" "\\1" input ${input})
  set(inputfilelist "${CURRENT_DATASET_DIR}/${TESTS_DIR}/${input}/${INPUT_LISTNAME}")
  set(expecteddump "${CURRENT_DATASET_DIR}/${TESTS_DIR}/${input}/${EXPECTED_DUMP}")
  set(testname "DICOM_Load_${input}")

  message(STATUS "Test case '${input}' : CTest test ${testname}")
  message(STATUS "  Input files in ${inputfilelist}")
  message(STATUS "  Expected mitk::Image dump in ${expecteddump}")

  # TODO provide error messages if input not valid

  file(STRINGS ${inputfilelist} rawDicomFiles)
  foreach(raw ${rawDicomFiles})
    set(fileWithFullPath ${CURRENT_DATASET_DIR}/${raw})
    list(APPEND dicomFiles ${fileWithFullPath})
  endforeach(raw ${rawDicomFiles})

  message(STATUS "  Load ${dicomFiles}")

  add_test(NAME ${testname}
           COMMAND ${VERIFY_DUMP_CMD} ${expecteddump} ${dicomFiles})
  mitkFunctionAddTestLabel(${testname}) #not yet in branch

endforeach(input allInputs)


