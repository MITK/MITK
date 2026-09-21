set(MODULE_TESTS
  mitkDICOMReaderConfiguratorTest.cpp
  mitkDICOMDCMTKTagScannerTest.cpp
  mitkDICOMFrameLayoutTest.cpp
  mitkDICOMMultiFrameReadTest.cpp
  mitkDICOMSimpleVolumeImportTest.cpp
  mitkDICOMSourceImageRelationTest.cpp
  mitkDICOMTagPathTest.cpp
  mitkDICOMPropertyTest.cpp
  mitkDICOMTagsOfInterestHelperTest.cpp
  mitkDICOMTimeUtilTest.cpp
)

set(MODULE_CUSTOM_TESTS
  mitkDICOMFileReaderTest.cpp
  mitkDICOMITKSeriesGDCMReaderBasicsTest.cpp
)

set(CPP_FILES
  mitkDICOMMultiFrameTestObject.cpp
  mitkDICOMNullFileReader.cpp
  mitkDICOMFilenameSorter.cpp
)
