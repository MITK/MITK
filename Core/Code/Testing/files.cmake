# tests with no extra command line parameter
set(MODULE_TESTS
  # IMPORTANT: If you plan to deactivate / comment out a test please write a bug number to the commented out line of code.
  #
  #     Example: #mitkMyTest #this test is commented out because of bug 12345
  #
  # It is important that the bug is open and that the test will be activated again before the bug is closed. This assures that
  # no test is forgotten after it was commented out. If there is no bug for your current problem, please add a new one and
  # mark it as critical.

  ################## DISABLED TESTS #################################################
  #mitkAbstractTransformGeometryTest.cpp #seems as tested class mitkExternAbstractTransformGeometry doesnt exist any more
  #mitkStateMachineContainerTest.cpp #rewrite test, indirect since no longer exported Bug 14529
  #mitkRegistrationBaseTest.cpp #tested class  mitkRegistrationBase doesn't exist any more
  #mitkSegmentationInterpolationTest.cpp #file doesn't exist!
  #mitkPipelineSmartPointerCorrectnessTest.cpp #file doesn't exist!
  #mitkITKThreadingTest.cpp #test outdated because itk::Semaphore was removed from ITK
  #mitkAbstractTransformPlaneGeometryTest.cpp #mitkVtkAbstractTransformPlaneGeometry doesn't exist any more
  #mitkTestUtilSharedLibrary.cpp #Linker problem with this test...
  #mitkTextOverlay2DSymbolsRenderingTest.cpp #Implementation of the tested feature is not finished yet. Ask Christoph or see bug 15104 for details.

  ################# RUNNING TESTS ###################################################
  mitkAccessByItkTest.cpp
  mitkCoreObjectFactoryTest.cpp
  mitkDataNodeTest.cpp
  mitkMaterialTest.cpp
  mitkActionTest.cpp
  mitkDispatcherTest.cpp
  mitkEnumerationPropertyTest.cpp
  mitkEventTest.cpp
  mitkFileReaderRegistryTest.cpp
  #mitkFileWriterRegistryTest.cpp
  mitkFocusManagerTest.cpp
  mitkGenericPropertyTest.cpp
  mitkGeometry3DTest.cpp
  mitkGeometry3DEqualTest.cpp
  mitkGeometryDataToSurfaceFilterTest.cpp
  mitkGlobalInteractionTest.cpp
  mitkImageCastTest.cpp
  mitkImageEqualTest.cpp
  mitkImageDataItemTest.cpp
  mitkImageGeneratorTest.cpp
  mitkIOUtilTest.cpp
  mitkBaseDataTest.cpp
  mitkImportItkImageTest.cpp
  mitkGrabItkImageMemoryTest.cpp
  mitkInstantiateAccessFunctionTest.cpp
  mitkInteractorTest.cpp
  mitkLevelWindowTest.cpp
  mitkMessageTest.cpp
  mitkPixelTypeTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPointSetTest.cpp
  mitkPointSetEqualTest.cpp
  mitkPointSetFileIOTest.cpp
  mitkPointSetOnEmptyTest.cpp
  mitkPointSetWriterTest.cpp
  mitkPointSetReaderTest.cpp
  mitkPointSetInteractorTest.cpp
  mitkPointSetPointOperationsTest.cpp
  mitkProgressBarTest.cpp
  mitkPropertyTest.cpp
  mitkPropertyListTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkSliceNavigationControllerTest.cpp
  mitkStateMachineTest.cpp
  mitkStateTest.cpp
  mitkSurfaceTest.cpp
  mitkSurfaceEqualTest.cpp
  mitkSurfaceToSurfaceFilterTest.cpp
  mitkTimeGeometryTest.cpp
  mitkProportionalTimeGeometryTest.cpp
  mitkTransitionTest.cpp
  mitkUndoControllerTest.cpp
  mitkVtkWidgetRenderingTest.cpp
  mitkVerboseLimitedLinearUndoTest.cpp
  mitkWeakPointerTest.cpp
  mitkTransferFunctionTest.cpp
  mitkStepperTest.cpp
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
  mitkAffineTransformBaseTest.cpp
  mitkPropertyAliasesTest.cpp
  mitkPropertyDescriptionsTest.cpp
  mitkPropertyExtensionsTest.cpp
  mitkPropertyFiltersTest.cpp
  mitkTinyXMLTest.cpp
  mitkRawImageFileReaderTest.cpp
  mitkInteractionEventTest.cpp
  mitkLookupTableTest.cpp
  mitkSTLFileReaderTest.cpp
  mitkPointTypeConversionTest.cpp
  mitkVectorTypeConversionTest.cpp
  mitkMatrixTypeConversionTest.cpp
  mitkArrayTypeConversionTest.cpp
  mitkSurfaceToImageFilterTest.cpp
  mitkBaseGeometryTest.cpp
  mitkImageToSurfaceFilterTest.cpp
  mitkEqualTest.cpp
  mitkLineTest.cpp
)

if(MITK_ENABLE_RENDERING_TESTING) #since mitkInteractionTestHelper is currently creating a vtkRenderWindow
set(MODULE_TESTS
  ${MODULE_TESTS}
  mitkPointSetDataInteractorTest.cpp
)
endif()



# test with image filename as an extra command line parameter
set(MODULE_IMAGE_TESTS
  mitkImageTimeSelectorTest.cpp #only runs on images
  mitkImageAccessorTest.cpp #only runs on images
)

set(MODULE_SURFACE_TESTS
  mitkSurfaceVtkWriterTest.cpp #only runs on surfaces
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
    mitkDataStorageTest.cpp
    mitkDicomSeriesReaderTest.cpp
    mitkDICOMLocaleTest.cpp
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
    mitkImageVtkMapper2DOpacityTransferFunctionTest.cpp
    mitkImageVtkMapper2DLookupTableTest.cpp
    mitkSurfaceVtkMapper3DTest
    mitkSurfaceVtkMapper3DTexturedSphereTest.cpp
    mitkSurfaceGLMapper2DColorTest.cpp
    mitkSurfaceGLMapper2DOpacityTest.cpp
    mitkVolumeCalculatorTest.cpp
    mitkLevelWindowManagerTest.cpp
    mitkPointSetVtkMapper2DTest.cpp
    mitkPointSetVtkMapper2DImageTest.cpp
    mitkPointSetVtkMapper2DGlyphTypeTest.cpp
    mitkPointSetVtkMapper2DTransformedPointsTest.cpp
    mitkVTKRenderWindowSizeTest.cpp
    mitkMultiComponentImageDataComparisonFilterTest.cpp
    mitkImageToItkTest.cpp
    mitkImageSliceSelectorTest.cpp
    mitkSurfaceDepthPeelingTest.cpp
    mitkSurfaceDepthSortingTest.cpp
)

set(RESOURCE_FILES
  Interactions/AddAndRemovePoints.xml
  Interactions/globalConfig.xml
  Interactions/StatemachineTest.xml
  Interactions/StatemachineConfigTest.xml
)
