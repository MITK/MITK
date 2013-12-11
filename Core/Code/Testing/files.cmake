
# tests with no extra command line parameter
set(MODULE_TESTS
  mitkAccessByItkTest.cpp
  mitkCoreObjectFactoryTest.cpp
  mitkMaterialTest.cpp
  mitkActionTest.cpp
  mitkDispatcherTest.cpp
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
  mitkImportItkImageTest.cpp
  mitkGrabItkImageMemoryTest.cpp
  mitkInstantiateAccessFunctionTest.cpp
  mitkInteractorTest.cpp
  #mitkITKThreadingTest.cpp
  mitkLevelWindowTest.cpp
  mitkMessageTest.cpp
  #mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPixelTypeTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPointSetFileIOTest.cpp
  mitkPointSetTest.cpp
  mitkPointSetWriterTest.cpp
  mitkPointSetReaderTest.cpp
  mitkPointSetInteractorTest.cpp
  mitkPropertyTest.cpp
  mitkPropertyListTest.cpp
  #mitkRegistrationBaseTest.cpp
  #mitkSegmentationInterpolationTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkSliceNavigationControllerTest.cpp
  mitkStateMachineTest.cpp
  ##mitkStateMachineContainerTest.cpp ## rewrite test, indirect since no longer exported Bug 14529
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
  mitkStepperTest.cpp
  itkTotalVariationDenoisingImageFilterTest.cpp
  mitkRenderingManagerTest.cpp
  vtkMitkThickSlicesFilterTest.cpp
  mitkNodePredicateSourceTest.cpp
  mitkVectorTest.cpp
  mitkClippedSurfaceBoundsCalculatorTest.cpp
  mitkExceptionTest.cpp
  mitkExtractSliceFilterTest.cpp
  mitkLogTest.cpp
  mitkImageDimensionConverterTest.cpp
  mitkLoggingAdapterTest.cpp
  mitkUIDGeneratorTest.cpp
  mitkShaderRepositoryTest.cpp
  mitkPlanePositionManagerTest.cpp
)

# test with image filename as an extra command line parameter
set(MODULE_IMAGE_TESTS
  mitkImageTimeSelectorTest.cpp #only runs on images
  mitkImageAccessorTest.cpp #only runs on images
  mitkDataNodeFactoryTest.cpp #runs on all types of data
)

set(MODULE_SURFACE_TESTS
  mitkSurfaceVtkWriterTest.cpp #only runs on surfaces
  mitkDataNodeFactoryTest.cpp #runs on all types of data
)

# list of images for which the tests are run
set(MODULE_TESTIMAGES
  US4DCyl.nrrd
  Pic3D.nrrd
  Pic2DplusT.nrrd
  BallBinary30x30x30.nrrd
  Png2D-bw.png
)
set(MODULE_TESTSURFACES
  binary.stl
  ball.stl
)

set(MODULE_CUSTOM_TESTS
    #mitkLabeledImageToSurfaceFilterTest.cpp
    #mitkExternalToolsTest.cpp
    mitkDataStorageTest.cpp
    mitkDataNodeTest.cpp
    mitkEventMapperTest.cpp
    mitkEventConfigTest.cpp
    mitkNodeDependentPointSetInteractorTest.cpp
    mitkStateMachineFactoryTest.cpp
    mitkPointSetLocaleTest.cpp
    mitkImageTest.cpp
    mitkImageWriterTest.cpp
    mitkImageVtkMapper2DTest.cpp
    mitkImageVtkMapper2DLevelWindowTest.cpp
    mitkImageVtkMapper2DOpacityTest.cpp
    mitkImageVtkMapper2DResliceInterpolationPropertyTest.cpp
    mitkImageVtkMapper2DColorTest.cpp
    mitkImageVtkMapper2DSwivelTest.cpp
    mitkImageVtkMapper2DTransferFunctionTest.cpp
    mitkIOUtilTest.cpp
    mitkSurfaceVtkMapper3DTest
    mitkSurfaceVtkMapper3DTexturedSphereTest.cpp
    mitkSurfaceGLMapper2DColorTest.cpp
    mitkSurfaceGLMapper2DOpacityTest.cpp
    mitkVolumeCalculatorTest.cpp
    mitkLevelWindowManagerTest.cpp
    mitkPointSetVtkMapper2DTest.cpp
    mitkPointSetVtkMapper2DImageTest.cpp
    mitkPointSetVtkMapper2DGlyphTypeTest.cpp
)

set(MODULE_RESOURCE_FILES
  Interactions/AddAndRemovePoints.xml
  Interactions/globalConfig.xml
  Interactions/StatemachineTest.xml
  Interactions/StatemachineConfigTest.xml
)

# Create an artificial module initializing class for
# the usServiceListenerTest.cpp
usFunctionGenerateModuleInit(testdriver_init_file
                             NAME ${MODULE_NAME}TestDriver
                             DEPENDS "Mitk"
                             VERSION "0.1.0"
                             EXECUTABLE
                            )

# Embed the resources
set(testdriver_resources )
usFunctionEmbedResources(testdriver_resources
                         EXECUTABLE_NAME ${MODULE_NAME}TestDriver
                         ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Resources
                         FILES ${MODULE_RESOURCE_FILES}
                        )

set(TEST_CPP_FILES ${testdriver_init_file} ${testdriver_resources})
