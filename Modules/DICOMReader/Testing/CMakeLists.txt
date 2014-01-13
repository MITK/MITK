MITK_CREATE_MODULE_TESTS()

file(GLOB_RECURSE tinyCTSlices ${MITK_DATA_DIR}/TinyCTAbdomen/1??)

#foreach(f ${tinyCTSlices})
#  message("  ${f}")
#endforeach()

mitkAddCustomModuleTest(mitkDICOMFileReaderTest_Basics mitkDICOMFileReaderTest ${tinyCTSlices})
mitkAddCustomModuleTest(mitkDICOMITKSeriesGDCMReaderBasicsTest_Basics mitkDICOMITKSeriesGDCMReaderBasicsTest ${tinyCTSlices})
