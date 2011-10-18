
# tests with no extra command line parameter
SET(MODULE_TESTS
  mitkAccessByItkTest.cpp
  mitkCoreObjectFactoryTest.cpp
  mitkPointSetWriterTest.cpp
  mitkMaterialTest.cpp
  mitkActionTest.cpp
  mitkEnumerationPropertyTest.cpp
  mitkEventTest.cpp
  mitkFocusManagerTest.cpp
  mitkGenericPropertyTest.cpp
  mitkGeometry3DTest.cpp
  mitkGeometryDataToSurfaceFilterTest.cpp
  mitkGlobalInteractionTest.cpp
  mitkImageDataItemTest.cpp
  #mitkImageMapper2DTest.cpp
  mitkImageGeneratorTest.cpp
  mitkBaseDataTest.cpp
  #mitkImageToItkTest.cpp
  mitkInstantiateAccessFunctionTest.cpp
  mitkInteractorTest.cpp
  mitkITKThreadingTest.cpp
  mitkLDAPFilterTest.cpp
  # mitkLevelWindowManagerTest.cpp
  mitkLevelWindowTest.cpp
  mitkMessageTest.cpp
  mitkModuleTest.cpp
  #mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPixelTypeTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPointSetFileIOTest.cpp
  mitkPointSetTest.cpp
  mitkPointSetInteractorTest.cpp
  mitkPropertyTest.cpp
  mitkPropertyListTest.cpp
  #mitkRegistrationBaseTest.cpp
  #mitkSegmentationInterpolationTest.cpp
  mitkServiceListenerTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkSliceNavigationControllerTest.cpp
  mitkStateMachineTest.cpp
  mitkStateTest.cpp
  mitkSurfaceTest.cpp
  mitkSurfaceToSurfaceFilterTest.cpp
  mitkTimeSlicedGeometryTest.cpp
  mitkTransitionTest.cpp
  mitkUndoControllerTest.cpp
  mitkVtkWidgetRenderingTest.cpp
  mitkVerboseLimitedLinearUndoTest.cpp
  mitkWeakPointerTest.cpp
  mitkTransferFunctionTest.cpp
  #mitkAbstractTransformGeometryTest.cpp
  #mitkPicFileIOTest.cpp
  mitkStepperTest.cpp
  itkTotalVariationDenoisingImageFilterTest.cpp
  mitkRenderingManagerTest.cpp
  vtkMitkThickSlicesFilterTest.cpp
  mitkNodePredicateSourceTest.cpp
  mitkVectorTest.cpp
  mitkClippedSurfaceBoundsCalculatorTest.cpp
)

# test with image filename as an extra command line parameter
SET(MODULE_IMAGE_TESTS
  mitkSurfaceVtkWriterTest.cpp
  mitkPicFileWriterTest.cpp
  #mitkImageSliceSelectorTest.cpp
  mitkImageTimeSelectorTest.cpp
  mitkPicFileReaderTest.cpp
  # mitkVtkPropRendererTest.cpp
  mitkDataNodeFactoryTest.cpp
  #mitkSTLFileReaderTest.cpp
)

# list of images for which the tests are run
SET(MODULE_TESTIMAGES
  US4DCyl.pic.gz
  Pic3D.pic.gz
  Pic2DplusT.pic.gz
  BallBinary30x30x30.pic.gz
  binary.stl
  ball.stl
)

SET(MODULE_CUSTOM_TESTS
    #mitkLabeledImageToSurfaceFilterTest.cpp
    #mitkExternalToolsTest.cpp
    mitkDataStorageTest.cpp
    mitkDataNodeTest.cpp
    mitkDicomSeriesReaderTest.cpp
    mitkDICOMLocaleTest.cpp
    mitkEventMapperTest.cpp
    mitkNodeDependentPointSetInteractorTest.cpp
    mitkStateMachineFactoryTest.cpp
    mitkPointSetLocaleTest.cpp
    mitkImageTest.cpp
    mitkImageWriterTest.cpp
)

# Create an artificial module initializing class for
# the mitkServiceListenerTest.cpp

SET(module_name_orig ${MODULE_NAME})
SET(module_libname_orig ${MODULE_LIBNAME})
SET(MODULE_NAME "${MODULE_NAME}TestDriver")
SET(MODULE_LIBNAME "")
SET(MODULE_DEPENDS_STR "Mitk")
SET(MODULE_PACKAGE_DEPENDS_STR "")
SET(MODULE_VERSION "0.1.0")
SET(MODULE_QT_BOOL "false")

SET(testdriver_init_file "${CMAKE_CURRENT_BINARY_DIR}/MitkTestDriver_init.cpp")
CONFIGURE_FILE("${MITK_SOURCE_DIR}/CMake/mitkModuleInit.cpp" ${testdriver_init_file} @ONLY)
SET(TEST_CPP_FILES ${testdriver_init_file})

SET(MODULE_NAME ${module_name_orig})
SET(MODULE_LIBNAME ${module_libname_orig})
