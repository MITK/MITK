MITK_CREATE_MODULE_TESTS()

file(GLOB_RECURSE tinyCTSlices ${MITK_DATA_DIR}/TinyCTAbdomen/1??)
file(GLOB_RECURSE sloppyDICOMfiles ${MITK_DATA_DIR}/SloppyDICOMFiles/1*)

#foreach(f ${sloppyDICOMfiles})
#  message("  ${f}")
#endforeach()

mitkAddCustomModuleTest(mitkDICOMFileReaderTest_Basics mitkDICOMFileReaderTest ${tinyCTSlices})
mitkAddCustomModuleTest(mitkDICOMITKSeriesGDCMReaderBasicsTest_Basics mitkDICOMITKSeriesGDCMReaderBasicsTest ${tinyCTSlices})
mitkAddCustomModuleTest(mitkDICOMSimpleVolumeImportTest_Basics mitkDICOMSimpleVolumeImportTest ${sloppyDICOMfiles})
