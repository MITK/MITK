INCLUDE_DIRECTORIES(${${KITNAME}_INCLUDE_DIRS} ${IPSEGMENTATION_INCLUDE_DIR})

ENABLE_TESTING()

# tests with no extra command line parameter
SET(${KITNAME}_TESTS
  mitkAbstractTransformGeometryTest.cpp
  mitkActionTest.cpp
  mitkAutoCropImageFilterTest.cpp
  mitkContourMapper2DTest.cpp
  mitkContourTest.cpp
  mitkDataStorageTest.cpp
  mitkDataTreeFilterTest.cpp
  mitkDataTreeTest.cpp
  mitkEnumerationPropertyTest.cpp
  mitkEventTest.cpp
  mitkGenericPropertyTest.cpp
  mitkGeometry3DTest.cpp
  mitkGeometryDataToSurfaceFilterTest.cpp
  mitkGlobalInteractionTest.cpp
  mitkImageDataItemTest.cpp
  mitkImageMapper2DTest.cpp
  mitkImageTest.cpp
  mitkImageToItkTest.cpp
  mitkInteractorTest.cpp
  mitkITKThreadingTest.cpp
  mitkLevelWindowManagerTest.cpp
  mitkLevelWindowTest.cpp
  mitkMeshTest.cpp
  mitkOrganTypePropertyTest.cpp
  mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPointSetFileIOTest.cpp
  mitkPointSetTest.cpp
  mitkPropertyListTest.cpp
  mitkRenderWindowTest.cpp
  mitkSegmentationInterpolationTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkSliceNavigationControllerTest.cpp
  mitkStateMachineFactoryTest.cpp
  mitkStateTest.cpp
  mitkStepperTest.cpp
  mitkSurfaceTest.cpp
  mitkSurfaceToImageFilterTest.cpp
  mitkTimeSlicedGeometryTest.cpp
  mitkToolManagerTest.cpp
  mitkTransitionTest.cpp
)

# test with image filename as an extra command line parameter
SET(${KITNAME}_IMAGE_TESTS
  mitkCompressedImageContainerTest.cpp  
  mitkCylindricToCartesianFilterTest.cpp
  mitkExtractImageFilterTest.cpp  
  mitkImageSliceSelectorTest.cpp
  mitkImageTimeSelectorTest.cpp
  mitkManualSegmentationToSurfaceFilterTest.cpp
  mitkOverwriteSliceImageFilterTest.cpp  
  mitkPicFileReaderTest.cpp
)

# list of images for which the tests are run
SET(${KITNAME}_TESTIMAGES
  US4DCyl.pic.gz
  Pic3D.pic.gz
  Pic2DplusT.pic.gz
  BallBinary30x30x30.pic.gz
  Png2D-bw.png
  binary.stl
)

CREATE_TEST_SOURCELIST(MITKTEST_SOURCE ${KITNAME}TestDriver.cpp 
  ${${KITNAME}_TESTS} ${${KITNAME}_IMAGE_TESTS}
)
ADD_EXECUTABLE(${KITNAME}TestDriver ${MITKTEST_SOURCE})
TARGET_LINK_LIBRARIES(${KITNAME}TestDriver ${${KITNAME}_CORE_LIBRARIES})

FOREACH( test ${${KITNAME}_TESTS} )
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  ADD_TEST(${TName} ${EXECUTABLE_OUTPUT_PATH}/MITKTestDriver ${TName})
ENDFOREACH( test )

# todo: this is probably needed somewhere else
# IF (CMAKE_CXX_COMPILER STREQUAL "cl")
#  SET_TARGET_PROPERTIES( MITKTestDriver PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt.lib")
# ENDIF (CMAKE_CXX_COMPILER STREQUAL "cl")

FOREACH(image ${${KITNAME}_TESTIMAGES} ${ADDITIONAL_TEST_IMAGES} )
  IF(EXISTS ${image})
    SET(IMAGE_FULL_PATH ${image})
  ELSE(EXISTS ${image})
    # todo: maybe search other paths as well
    SET(IMAGE_FULL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Data/${image})
  ENDIF(EXISTS ${image})

  IF(EXISTS ${IMAGE_FULL_PATH})
  FOREACH( test ${${KITNAME}_IMAGE_TESTS} )
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
    GET_FILENAME_COMPONENT(ImageName ${IMAGE_FULL_PATH} NAME)
    ADD_TEST(${TName}_${ImageName} ${EXECUTABLE_OUTPUT_PATH}/MITKTestDriver ${TName} ${IMAGE_FULL_PATH})
  ENDFOREACH( test )
  ELSE(EXISTS ${IMAGE_FULL_PATH})
    MESSAGE("!!!!! No such file: ${IMAGE_FULL_PATH} !!!!!")
  ENDIF(EXISTS ${IMAGE_FULL_PATH})
ENDFOREACH( image )

# IF (CMAKE_CXX_COMPILER STREQUAL "cl")
#   SET_TARGET_PROPERTIES( MITKImageTestDriver PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt.lib")
# ENDIF (CMAKE_CXX_COMPILER STREQUAL "cl")

