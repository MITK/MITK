MITK_CREATE_MODULE_TESTS(LABELS MITK-Modules)

mitkAddCustomModuleTest(mitkPicFileReaderTest_emptyFile mitkPicFileReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/emptyFile.pic)
mitkAddCustomModuleTest(mitkPicFileReaderTest_emptyGzipFile mitkPicFileReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/emptyFile.pic.gz)

add_test(mitkPicFileReaderTest_emptyGzipFile ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkPicFileReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/emptyFile.pic.gz)
set_property(TEST mitkPicFileReaderTest_emptyGzipFile PROPERTY LABELS MITK-Modules)
