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
  mitkFileReaderRegistryTest.cpp
  #mitkFileWriterRegistryTest.cpp
  mitkFloatToStringTest.cpp
  mitkGenericPropertyTest.cpp
  mitkGeometry3DTest.cpp
  mitkGeometry3DEqualTest.cpp
  mitkGeometryDataIOTest.cpp
  mitkGeometryDataToSurfaceFilterTest.cpp
  mitkImageCastTest.cpp
  mitkImageEqualTest.cpp
  mitkImageDataItemTest.cpp
  mitkImageGeneratorTest.cpp
  mitkIOUtilTest.cpp
  mitkBaseDataTest.cpp
  mitkImportItkImageTest.cpp
  mitkGrabItkImageMemoryTest.cpp
  mitkInstantiateAccessFunctionTest.cpp
  mitkLevelWindowTest.cpp
  mitkMessageTest.cpp
  mitkPixelTypeTest.cpp
  mitkPlaneGeometryTest.cpp
  mitkPointSetTest.cpp
  mitkPointSetEqualTest.cpp
  mitkPointSetFileIOTest.cpp
  mitkPointSetOnEmptyTest.cpp
  mitkPointSetLocaleTest.cpp
  mitkPointSetWriterTest.cpp
  mitkPointSetReaderTest.cpp
  mitkPointSetPointOperationsTest.cpp
  mitkProgressBarTest.cpp
  mitkPropertyTest.cpp
  mitkPropertyListTest.cpp
  mitkPropertyPersistenceTest.cpp
  mitkPropertyPersistenceInfoTest.cpp
  mitkPropertyRelationRuleBaseTest.cpp
  mitkPropertyRelationsTest.cpp
  mitkSlicedGeometry3DTest.cpp
  mitkSliceNavigationControllerTest.cpp
  mitkSurfaceTest.cpp
  mitkSurfaceEqualTest.cpp
  mitkSurfaceToSurfaceFilterTest.cpp
  mitkTimeGeometryTest.cpp
  mitkProportionalTimeGeometryTest.cpp
  mitkUndoControllerTest.cpp
  mitkVtkWidgetRenderingTest.cpp
  mitkVerboseLimitedLinearUndoTest.cpp
  mitkWeakPointerTest.cpp
  mitkTransferFunctionTest.cpp
  mitkStepperTest.cpp
  mitkRenderingManagerTest.cpp
  mitkCompositePixelValueToStringTest.cpp
  vtkMitkThickSlicesFilterTest.cpp
  mitkNodePredicateSourceTest.cpp
  mitkNodePredicateDataPropertyTest.cpp
  mitkNodePredicateFunctionTest.cpp
  mitkVectorTest.cpp
  mitkClippedSurfaceBoundsCalculatorTest.cpp
  mitkExceptionTest.cpp
  mitkExtractSliceFilterTest.cpp
  mitkLogTest.cpp
  mitkImageDimensionConverterTest.cpp
  mitkLoggingAdapterTest.cpp
  mitkUIDGeneratorTest.cpp
  mitkPlanePositionManagerTest.cpp
  mitkAffineTransformBaseTest.cpp
  mitkPropertyAliasesTest.cpp
  mitkPropertyDescriptionsTest.cpp
  mitkPropertyExtensionsTest.cpp
  mitkPropertyFiltersTest.cpp
  mitkPropertyKeyPathTest.cpp
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
  mitkArbitraryTimeGeometryTest
  mitkItkImageIOTest.cpp
  mitkRotatedSlice4DTest.cpp
  mitkLevelWindowManagerCppUnitTest.cpp
  mitkVectorPropertyTest.cpp
  mitkTemporoSpatialStringPropertyTest.cpp
  mitkPropertyNameHelperTest.cpp
  mitkNodePredicateGeometryTest.cpp
  mitkPreferenceListReaderOptionsFunctorTest.cpp
  mitkGenericIDRelationRuleTest.cpp
  mitkSourceImageRelationRuleTest.cpp
)

if(MITK_ENABLE_RENDERING_TESTING)
set(MODULE_TESTS
  ${MODULE_TESTS}
  mitkPlaneGeometryDataMapper2DTest.cpp
  mitkPointSetDataInteractorTest.cpp #since mitkInteractionTestHelper is currently creating a vtkRenderWindow
  mitkSurfaceVtkMapper2DTest.cpp #new rendering test in CppUnit style
  mitkSurfaceVtkMapper2D3DTest.cpp # comparisons/consistency 2D/3D
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
set(MODULE_TESTIMAGE
  US4DCyl.nrrd
  Pic3D.nrrd
  Pic2DplusT.nrrd
  BallBinary30x30x30.nrrd
  Png2D-bw.png
)
set(MODULE_TESTSURFACE
  binary.stl
  ball.stl
)

set(MODULE_CUSTOM_TESTS
    mitkDataStorageTest.cpp
    mitkDataNodeTest.cpp
    mitkEventConfigTest.cpp
    mitkPointSetLocaleTest.cpp
    mitkImageTest.cpp
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
)

# Currently not working on windows because of a rendering timing issue
# see bug 18083 for details
if(NOT WIN32)
 set(MODULE_CUSTOM_TESTS ${MODULE_CUSTOM_TESTS} mitkSurfaceDepthSortingTest.cpp)
endif()

set(RESOURCE_FILES
  Interactions/AddAndRemovePoints.xml
  Interactions/globalConfig.xml
  Interactions/StatemachineTest.xml
  Interactions/StatemachineConfigTest.xml
)
