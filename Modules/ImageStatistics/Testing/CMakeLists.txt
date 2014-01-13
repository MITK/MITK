<<<<<<< HEAD
MITK_CREATE_MODULE_TESTS()

mitkAddCustomModuleTest(mitkImageStatisticsHotspotTest_Case1 mitkImageStatisticsHotspotTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/Hotspot_Case1.xml)
=======
MITK_CREATE_MODULE_TESTS(EXTRA_DEPENDS DICOMReader)
>>>>>>> origin/bug-16744-dcm-refactor
