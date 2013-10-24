
# tests with no extra command line parameter
set(MODULE_TESTS
  itkTotalVariationDenoisingImageFilterTest.cpp
  #mitkAbstractTransformGeometryTest.cpp
  mitkAccessByItkTest.cpp
  mitkActionTest.cpp
  mitkAffineTransformBaseTest.cpp
  mitkBaseDataTest.cpp
  mitkClippedSurfaceBoundsCalculatorTest.cpp
  mitkCoreObjectFactoryTest.cpp
  mitkDICOMLocaleTest.cpp
  mitkDispatcherTest.cpp
  mitkEnumerationPropertyTest.cpp
  mitkEventTest.cpp
  mitkExceptionTest.cpp
  mitkExtractSliceFilterTest.cpp
  mitkFocusManagerTest.cpp
  mitkGenericPropertyTest.cpp
  mitkGeometry2DTest.cpp
  mitkGeometry3DEqualTest.cpp
  mitkGeometry3DTest.cpp
  mitkGeometryDataToSurfaceFilterTest.cpp
  mitkGlobalInteractionTest.cpp
  mitkGrabItkImageMemoryTest.cpp
  mitkImageDataItemTest.cpp
  mitkImageDimensionConverterTest.cpp
  mitkImageEqualTest.cpp
  mitkImageGeneratorTest.cpp
  #mitkImageMapper2DTest.cpp
  #mitkImageToItkTest.cpp
  mitkImportItkImageTest.cpp
  mitkInstantiateAccessFunctionTest.cpp
  mitkInteractorTest.cpp
  mitkIOUtilTest.cpp
  #mitkITKThreadingTest.cpp
  mitkLevelWindowTest.cpp
  mitkLoggingAdapterTest.cpp
  mitkLogTest.cpp
  mitkMaterialTest.cpp
  mitkMessageTest.cpp
  mitkNodePredicateSourceTest.cpp
  #mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPixelTypeTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPlanePositionManagerTest.cpp
  mitkPointSetEqualTest.cpp
  mitkPointSetFileIOTest.cpp
  mitkPointSetInteractorTest.cpp
  mitkPointSetReaderTest.cpp
  mitkPointSetTest.cpp
  mitkPointSetWriterTest.cpp
  mitkPropertyAliasesTest.cpp
  mitkPropertyDescriptionsTest.cpp
  mitkPropertyExtensionsTest.cpp
  mitkPropertyFiltersTest.cpp
  mitkPropertyListTest.cpp
  mitkPropertyTest.cpp
  #mitkRegistrationBaseTest.cpp
  mitkRenderingManagerTest.cpp
  #mitkSegmentationInterpolationTest.cpp
  mitkShaderRepositoryTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkSliceNavigationControllerTest.cpp
  ##mitkStateMachineContainerTest.cpp ## rewrite test, indirect since no longer exported Bug 14529
  mitkStateMachineTest.cpp
  mitkStateTest.cpp
  mitkStepperTest.cpp
  mitkSurfaceEqualTest.cpp
  mitkSurfaceTest.cpp
  mitkSurfaceToSurfaceFilterTest.cpp
  mitkTimeGeometryTest.cpp
  mitkTransferFunctionTest.cpp
  mitkTransitionTest.cpp
  mitkUIDGeneratorTest.cpp
  mitkUndoControllerTest.cpp
  mitkVectorTest.cpp
  mitkVerboseLimitedLinearUndoTest.cpp
  mitkVtkWidgetRenderingTest.cpp
  mitkWeakPointerTest.cpp
  vtkMitkThickSlicesFilterTest.cpp
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
    mitkDataNodeTest.cpp
    mitkDataStorageTest.cpp
    mitkDicomSeriesReaderTest.cpp
    mitkEventConfigTest.cpp
    mitkEventMapperTest.cpp
    #mitkExternalToolsTest.cpp
    mitkImageTest.cpp
    mitkImageVtkMapper2DColorTest.cpp
    mitkImageVtkMapper2DLevelWindowTest.cpp
    mitkImageVtkMapper2DLookupTableTest.cpp
    mitkImageVtkMapper2DOpacityTest.cpp
    mitkImageVtkMapper2DResliceInterpolationPropertyTest.cpp
    mitkImageVtkMapper2DSwivelTest.cpp
    mitkImageVtkMapper2DTest.cpp
    mitkImageVtkMapper2DTransferFunctionTest.cpp
    mitkImageWriterTest.cpp
    #mitkLabeledImageToSurfaceFilterTest.cpp
    mitkLabelOverlay3DRendering2DTest.cpp
    mitkLabelOverlay3DRendering3DTest.cpp
    mitkLevelWindowManagerTest.cpp
    mitkNodeDependentPointSetInteractorTest.cpp
    mitkPointSetLocaleTest.cpp
    mitkPointSetVtkMapper2DGlyphTypeTest.cpp
    mitkPointSetVtkMapper2DImageTest.cpp
    mitkPointSetVtkMapper2DTest.cpp
    mitkPointSetVtkMapper2DTransformedPointsTest.cpp
    mitkStateMachineFactoryTest.cpp
    mitkSurfaceGLMapper2DColorTest.cpp
    mitkSurfaceGLMapper2DOpacityTest.cpp
    mitkSurfaceVtkMapper3DTest
    mitkSurfaceVtkMapper3DTexturedSphereTest.cpp
    mitkTextOverlay2DLayouterRenderingTest.cpp
    mitkTextOverlay2DRenderingTest.cpp
    mitkTextOverlay3DColorRenderingTest.cpp
    mitkTextOverlay3DRendering2DTest.cpp
    mitkTextOverlay3DRendering3DTest.cpp
    mitkVolumeCalculatorTest.cpp
    mitkVTKRenderWindowSizeTest.cpp
)

if (${VTK_MAJOR_VERSION} VERSION_LESS 6) # test can be removed with VTK 6
  set(MODULE_TESTS ${MODULE_TESTS} mitkVTKRenderWindowSizeTest.cpp)
endif()

set(MODULE_RESOURCE_FILES
  Interactions/AddAndRemovePoints.xml
  Interactions/globalConfig.xml
  Interactions/StatemachineTest.xml
  Interactions/StatemachineConfigTest.xml
)

# Create an artificial module initializing class for
# the usServiceListenerTest.cpp
usFunctionGenerateExecutableInit(testdriver_init_file
                                 IDENTIFIER ${MODULE_NAME}TestDriver
                                )

# Embed the resources
set(testdriver_resources )
usFunctionEmbedResources(testdriver_resources
                         EXECUTABLE_NAME ${MODULE_NAME}TestDriver
                         ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Resources
                         FILES ${MODULE_RESOURCE_FILES}
                        )

set(TEST_CPP_FILES ${testdriver_init_file} ${testdriver_resources})
