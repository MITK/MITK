INCLUDE(${MITK_SOURCE_DIR}/CMake/TestingHelpers.cmake)
INCLUDE_DIRECTORIES(${${KITNAME}_INCLUDE_DIRS} ${IPSEGMENTATION_INCLUDE_DIR} ${ANN_INCLUDE_DIR})

ENABLE_TESTING()

# tests with no extra command line parameter
SET(${KITNAME}_TESTS
  mitkMaterialPropertyTest.cpp
  mitkDataTreeNodeTest.cpp
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
  mitkMessageTest.cpp
  mitkOrganTypePropertyTest.cpp
  mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPointLocatorTest.cpp  
  mitkPointSetFileIOTest.cpp
  mitkPointSetTest.cpp
  mitkPropertyListTest.cpp
  mitkRigidRegistrationPresetTest.cpp
  mitkSegmentationInterpolationTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkSliceNavigationControllerTest.cpp
  mitkStateMachineTest.cpp
  mitkStateMachineFactoryTest.cpp
  mitkStateTest.cpp
  mitkStepperTest.cpp
  mitkSurfaceTest.cpp
  mitkTimeSlicedGeometryTest.cpp
  mitkToolManagerTest.cpp
  mitkTransitionTest.cpp
  mitkUndoControllerTest.cpp
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
  mitkSurfaceToImageFilterTest.cpp
  mitkVtkPropRendererTest.cpp
)

# list of images for which the tests are run
SET(${KITNAME}_TESTIMAGES
  US4DCyl.pic.gz
  Pic3D.pic.gz
  Pic2DplusT.pic.gz
  BallBinary30x30x30.pic.gz
  Png2D-bw.png
  binary.stl
  ball.stl
)

SET(${KITNAME}_CUSTOM_TESTS 
    mitkLabeledImageToSurfaceFilterTest.cpp
    mitkExternalToolsTest.cpp
)

MITK_CREATE_DEFAULT_TESTS()

ADD_TEST(mitkLabeledImageToSurfaceFilterTest ${EXECUTABLE_OUTPUT_PATH}/${KITNAME}TestDriver mitkLabeledImageToSurfaceFilterTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/BallBinary30x30x30.pic.gz)
