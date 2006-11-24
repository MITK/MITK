INCLUDE_DIRECTORIES(${MITK_INCLUDE_DIRS})

ENABLE_TESTING()

# tests with no extra command line parameter
SET(TESTS_WITHOUT_INPUT
  MITKTestDriver.cpp
  mitkAbstractTransformGeometryTest.cpp
  mitkActionTest.cpp
  mitkAutoCropImageFilterTest.cpp
  mitkContourTest.cpp
  mitkContourMapper2DTest.cpp
  mitkDataTreeTest.cpp
  mitkDataTreeFilterTest.cpp
  mitkEventTest.cpp
  mitkEnumerationPropertyTest.cpp
  mitkGenericPropertyTest.cpp
  mitkGeometry3DTest.cpp
  mitkGlobalInteractionTest.cpp
  mitkImageTest.cpp
  mitkImageDataItemTest.cpp
  mitkImageToItkTest.cpp
  mitkImageMapper2DTest.cpp
  mitkLevelWindowTest.cpp
  mitkLevelWindowManagerTest.cpp
  mitkMeshTest.cpp
  mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPointSetTest.cpp
  mitkPropertyListTest.cpp
  mitkRenderWindowTest.cpp
  mitkSliceNavigationControllerTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkStateTest.cpp
  mitkStepperTest.cpp
  mitkSurfaceTest.cpp
  mitkManualSegmentationToSurfaceFilterTest.cpp
  mitkSurfaceToImageFilterTest.cpp
  mitkTransitionTest.cpp
  mitkGeometryDataToSurfaceFilterTest.cpp
  mitkITKThreadingTest.cpp
)

# test with image filename as an extra command line parameter
CREATE_TEST_SOURCELIST(MITKIMAGETEST_SOURCE
  MITKImageTestDriver.cpp
  mitkPicFileReaderTest.cpp
  mitkImageTimeSelectorTest.cpp
  mitkImageSliceSelectorTest.cpp
  mitkCylindricToCartesianFilterTest.cpp
)

# list of images for which the tests are run
SET(MITK_TESTIMAGES
  US4DCyl.pic.gz
  Pic3D.pic.gz
  Pic2DplusT.pic.gz
)

CREATE_TEST_SOURCELIST(MITKTEST_SOURCE
  ${TESTS_WITHOUT_INPUT}
)
ADD_EXECUTABLE(MITKTestDriver ${MITKTEST_SOURCE})
TARGET_LINK_LIBRARIES(MITKTestDriver ${MITK_CORE_LIBRARIES})

SET(MITKTestToRun ${MITKTEST_SOURCE})
REMOVE(MITKTestToRun MITKTestDriver.cpp)

FOREACH( test ${MITKTestToRun} )
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  ADD_TEST(${TName} ${EXECUTABLE_OUTPUT_PATH}/MITKTestDriver ${TName} ${PROJECT_SOURCE_DIR})
ENDFOREACH( test )

# todo: this is probably needed somewhere else
IF (CMAKE_CXX_COMPILER STREQUAL "cl")
  SET_TARGET_PROPERTIES( MITKTestDriver PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt.lib")
ENDIF (CMAKE_CXX_COMPILER STREQUAL "cl")

ADD_EXECUTABLE(MITKImageTestDriver ${MITKIMAGETEST_SOURCE})
TARGET_LINK_LIBRARIES(MITKImageTestDriver ${MITK_CORE_LIBRARIES})

SET(MITKImageTestToRun ${MITKIMAGETEST_SOURCE})
REMOVE(MITKImageTestToRun MITKImageTestDriver.cpp)
FOREACH(image ${MITK_TESTIMAGES} )
  FOREACH( test ${MITKImageTestToRun} )
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
#    STRING(REGEX REPLACE "/" "_" imageComp ${image})
    ADD_TEST(${TName}_${image} ${EXECUTABLE_OUTPUT_PATH}/MITKImageTestDriver ${TName} ${PROJECT_SOURCE_DIR}/${image})
  ENDFOREACH( test )
ENDFOREACH( image )
IF(ADDITIONAL_TEST_IMAGES)
FOREACH(image ${ADDITIONAL_TEST_IMAGES})
  FOREACH( test ${MITKImageTestToRun} )
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
    GET_FILENAME_COMPONENT(ImageName ${image} NAME)
    #    STRING(REGEX REPLACE "/" "_" imageComp ${image})
    ADD_TEST(${TName}_${ImageName} ${EXECUTABLE_OUTPUT_PATH}/MITKImageTestDriver ${TName} ${image})
  ENDFOREACH( test )
ENDFOREACH( image )
ENDIF(ADDITIONAL_TEST_IMAGES)

IF (CMAKE_CXX_COMPILER STREQUAL "cl")
  SET_TARGET_PROPERTIES( MITKImageTestDriver PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt.lib")
ENDIF (CMAKE_CXX_COMPILER STREQUAL "cl")

