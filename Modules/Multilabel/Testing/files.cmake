set(MODULE_TESTS
    mitkDICOMSegFileContentTestHelpersTest.cpp
    mitkDICOMSegPerFrameSliceKeyingTest.cpp
    mitkDICOMSegmentationIOReaderTest.cpp
    mitkDICOMSegmentationIORoundTripTest.cpp
    mitkDICOMSegmentationIOWriterTest.cpp
    mitkDICOMSegmentationPropertyHelperTest.cpp
    mitkLabelTest.cpp
    mitkLabelSetImageTest.cpp
    mitkLabelSuggestionHelperTest.cpp
    mitkLegacyLabelSetImageIOTest.cpp
    mitkMultiLabelIOHelperTest.cpp
    mitkMultiLabelPredicateHelperTest.cpp
    mitkMultiLabelSegmentationIOTest.cpp
    mitkMultiLabelSegmentationStackReaderTest.cpp
    mitkMultiLabelSegmentationStackWriterTest.cpp
    mitkMultiLabelSurfaceExtractionSchedulerTest.cpp
    mitkMultiLabelSurfaceNetsExtractorTest.cpp
    mitkSegIOTestComparatorTest.cpp
    mitkSegSourceImageRelationRoundTripTest.cpp
    mitkSegSourceImageRelationRuleTest.cpp
    mitkTransferLabelTest.cpp
)

set(MODULE_RENDERING_TESTS
    mitkLabelSetImageVtkMapper2DTest.cpp
    mitkMultiLabelSegmentationVtkMapper3DTest.cpp
)

set(MODULE_CUSTOM_TESTS
    mitkDICOMSegmentationIOFileContentTest.cpp
    mitkDICOMSegmentationIORegressionTest.cpp
    mitkDICOMSegmentationIONonZOrderedTest.cpp
    mitkDICOMSegmentationIOLabelmapRegressionTest.cpp
    mitkDICOMSegmentationLegacyMigrationTest.cpp
    mitkDICOMSegSourceReferenceTest.cpp
)

