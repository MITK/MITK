INCLUDE(${MITK_SOURCE_DIR}/CMake/TestingHelpers.cmake)

INCLUDE_DIRECTORIES(${${KITNAME}_INCLUDE_DIRS} ${IPSEGMENTATION_INCLUDE_DIR} ${ANN_INCLUDE_DIR})

ENABLE_TESTING()

# tests with no extra command line parameter
SET(${KITNAME}_TESTS
  mitkPointSetWriterTest.cpp
  mitkMaterialPropertyTest.cpp
  mitkDataTreeNodeTest.cpp
  mitkActionTest.cpp
  mitkDataStorageTest.cpp
  mitkEnumerationPropertyTest.cpp
  mitkEventTest.cpp
  mitkGenericPropertyTest.cpp
  mitkGeometry3DTest.cpp
  mitkGeometryDataToSurfaceFilterTest.cpp
  mitkGlobalInteractionTest.cpp
  mitkImageDataItemTest.cpp
  #mitkImageMapper2DTest.cpp
  mitkImageTest.cpp
  #mitkImageToItkTest.cpp
  mitkInteractorTest.cpp
  mitkITKThreadingTest.cpp
  # mitkLevelWindowManagerTest.cpp
  mitkLevelWindowTest.cpp
  mitkMessageTest.cpp
  #mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPointSetFileIOTest.cpp
  mitkPointSetTest.cpp
  mitkPropertyListTest.cpp
  #mitkRegistrationBaseTest.cpp
  #mitkSegmentationInterpolationTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkSliceNavigationControllerTest.cpp
  mitkStateMachineTest.cpp
  mitkStateMachineFactoryTest.cpp
  #mitkStateTest.cpp
  mitkSurfaceTest.cpp
  mitkTimeSlicedGeometryTest.cpp
  mitkTransitionTest.cpp
  mitkUndoControllerTest.cpp
  mitkVtkWidgetRenderingTest.cpp
  mitkTransferFunctionTest.cpp
  #mitkAbstractTransformGeometryTest.cpp
  #mitkPicFileIOTest.cpp
  #mitkStepperTest.cpp
  
)

# test with image filename as an extra command line parameter
SET(${KITNAME}_IMAGE_TESTS
  mitkSurfaceVtkWriterTest.cpp
  mitkPicFileWriterTest.cpp
  mitkImageWriterTest.cpp
  #mitkImageSliceSelectorTest.cpp
  mitkImageTimeSelectorTest.cpp
  mitkPicFileReaderTest.cpp
  # mitkVtkPropRendererTest.cpp
)

# list of images for which the tests are run
SET(${KITNAME}_TESTIMAGES
  US4DCyl.pic.gz
  Pic3D.pic.gz
  Pic2DplusT.pic.gz
  BallBinary30x30x30.pic.gz
  binary.stl
  ball.stl
)

SET(${KITNAME}_CUSTOM_TESTS 
    #mitkLabeledImageToSurfaceFilterTest.cpp
    #mitkExternalToolsTest.cpp
)

MITK_CREATE_DEFAULT_TESTS()

TARGET_LINK_LIBRARIES(${KITNAME}TestDriver tinyxml)

# ADD_TEST(mitkLabeledImageToSurfaceFilterTest ${EXECUTABLE_OUTPUT_PATH}/${KITNAME}TestDriver mitkLabeledImageToSurfaceFilterTest ${CMAKE_CURRENT_SOURCE_DIR}/Data/BallBinary30x30x30.pic.gz)
