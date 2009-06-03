INCLUDE(${MITK_SOURCE_DIR}/CMake/TestingHelpers.cmake)
INCLUDE_DIRECTORIES(${${KITNAME}_INCLUDE_DIRS} ${IPSEGMENTATION_INCLUDE_DIR} ${ANN_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${MITK_IGT_INCLUDE_DIRS})
ENABLE_TESTING()

# tests with no extra command line parameter
SET(${KITNAME}_TESTS
   mitkCameraVisualizationTest.cpp 
   mitkClaronInterfaceTest.cpp
   mitkClaronToolTest.cpp
   mitkClaronTrackingDeviceTest.cpp
   mitkInternalTrackingToolTest.cpp
   mitkNavigationDataDisplacementFilterTest.cpp
   mitkNavigationDataLandmarkTransformFilterTest.cpp
   mitkNavigationDataObjectVisualizationFilterTest.cpp
   mitkNavigationDataSourceTest.cpp
   mitkNavigationDataToMessageFilterTest.cpp
   mitkNavigationDataToNavigationDataFilterTest.cpp
   mitkNavigationDataToPointSetFilterTest.cpp
   mitkNavigationDataTransformFilterTest.cpp  
   mitkNDIPassiveToolTest.cpp
   mitkTimeStampTest.cpp
   mitkTrackingVolumeTest.cpp
)



# test with image filename as an extra command line parameter
SET(${KITNAME}_IMAGE_TESTS

)

# list of images for which the tests are run
SET(${KITNAME}_TESTIMAGES
 
)

SET(${KITNAME}_CUSTOM_TESTS 
   
)

MITK_CREATE_DEFAULT_TESTS()


TARGET_LINK_LIBRARIES(${KITNAME}TestDriver mitkIGT tinyxml)

