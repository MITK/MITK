INCLUDE_DIRECTORIES(${${KITNAME}_INCLUDE_DIRS})

ENABLE_TESTING()

# tests with no extra command line parameter
SET(${KITNAME}_TESTS
  mitkAbstractTransformGeometryTest.cpp
  mitkActionTest.cpp
  mitkAutoCropImageFilterTest.cpp
  mitkContourTest.cpp
  mitkContourMapper2DTest.cpp
  mitkDataStorageTest.cpp
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
  mitkInteractorTest.cpp
  mitkLevelWindowTest.cpp
  mitkLevelWindowManagerTest.cpp
  mitkMeshTest.cpp
  mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPointSetTest.cpp
  mitkPointSetFileIOTest.cpp
  mitkPropertyListTest.cpp
  mitkRenderWindowTest.cpp
  mitkSliceNavigationControllerTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkStateMachineFactoryTest.cpp
  mitkStateTest.cpp
  mitkStepperTest.cpp
  mitkSurfaceTest.cpp
  mitkSurfaceToImageFilterTest.cpp
  mitkTimeSlicedGeometryTest.cpp
  mitkTransitionTest.cpp
  mitkGeometryDataToSurfaceFilterTest.cpp
  mitkITKThreadingTest.cpp
)

# test with image filename as an extra command line parameter
SET(${KITNAME}_IMAGE_TESTS
  mitkPicFileReaderTest.cpp
  mitkImageTimeSelectorTest.cpp
  mitkImageSliceSelectorTest.cpp
  mitkCylindricToCartesianFilterTest.cpp
  mitkManualSegmentationToSurfaceFilterTest.cpp
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

