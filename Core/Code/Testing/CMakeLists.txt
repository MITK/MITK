# The core tests need relaxed compiler flags...
# TODO fix core tests to compile without these additional no-error flags
if(MSVC_VERSION)
  # disable deprecated warnings (they would lead to errors)
  mitkFunctionCheckCAndCXXCompilerFlags("/wd4996" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
else()
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=deprecated" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=deprecated-declarations" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

MITK_CREATE_MODULE_TESTS(US_MODULE)
mitk_use_modules(TARGET ${TESTDRIVER} PACKAGES ITK|ITKThresholding)

mitkAddCustomModuleTest(mitkVolumeCalculatorTest_Png2D-bw mitkVolumeCalculatorTest
                        ${MITK_DATA_DIR}/Png2D-bw.png
                        ${MITK_DATA_DIR}/Pic2DplusT.nrrd
)

mitkAddCustomModuleTest(mitkEventMapperTest_Test1And2 mitkEventMapperTest
                        ${MITK_DATA_DIR}/TestStateMachine1.xml
                        ${MITK_DATA_DIR}/TestStateMachine2.xml
)

mitkAddCustomModuleTest(mitkEventConfigTest_CreateObjectInDifferentWays mitkEventConfigTest
                        ${MITK_SOURCE_DIR}/Core/Code/Testing/Resources/Interactions/StatemachineConfigTest.xml
)

mitkAddCustomModuleTest(mitkNodeDependentPointSetInteractorTest mitkNodeDependentPointSetInteractorTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd
                        ${MITK_DATA_DIR}/BallBinary30x30x30.nrrd
)

mitkAddCustomModuleTest(mitkDataStorageTest_US4DCyl mitkDataStorageTest
                        ${MITK_DATA_DIR}/US4DCyl.nrrd
)

mitkAddCustomModuleTest(mitkStateMachineFactoryTest_TestStateMachine1_2 mitkStateMachineFactoryTest
                        ${MITK_DATA_DIR}/TestStateMachine1.xml
                        ${MITK_DATA_DIR}/TestStateMachine2.xml
)

mitkAddCustomModuleTest(mitkDicomSeriesReaderTest_CTImage mitkDicomSeriesReaderTest
                        ${MITK_DATA_DIR}/TinyCTAbdomen
                        ${MITK_DATA_DIR}/DICOMReader/Broken-Series
)

mitkAddCustomModuleTest(mitkPointSetReaderTest mitkPointSetReaderTest
                        ${MITK_DATA_DIR}/PointSetReaderTestData.mps
)

mitkAddCustomModuleTest(mitkImageTest_4DImageData mitkImageTest
                        ${MITK_DATA_DIR}/US4DCyl.nrrd
)

mitkAddCustomModuleTest(mitkImageTest_2D+tImageData mitkImageTest
                        ${MITK_DATA_DIR}/Pic2DplusT.nrrd
)

mitkAddCustomModuleTest(mitkImageTest_3DImageData mitkImageTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd
)

mitkAddCustomModuleTest(mitkImageEqualTest mitkImageEqualTest)

mitkAddCustomModuleTest(mitkImageTest_brainImage mitkImageTest
                        ${MITK_DATA_DIR}/brain.mhd
)

mitkAddCustomModuleTest(mitkImageTest_3DImageData mitkImageGeneratorTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd
)

mitkAddCustomModuleTest(mitkLevelWindowManagerTest mitkLevelWindowManagerTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd
)

mitkAddCustomModuleTest(mitkMultiComponentImageDataComparisonFilterTest mitkMultiComponentImageDataComparisonFilterTest
                        ${MITK_DATA_DIR}/NrrdWritingTestImage.jpg
)

mitkAddCustomModuleTest(mitkImageToItkTest mitkImageToItkTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd
)

mitkAddCustomModuleTest(mitkImageSliceSelectorTest mitkImageSliceSelectorTest
                        ${MITK_DATA_DIR}/Pic2DplusT.nrrd
)


if(MITK_ENABLE_RENDERING_TESTING) ### since the rendering test's do not run in ubuntu, yet, we build them only for other systems or if the user explicitly sets the variable MITK_ENABLE_RENDERING_TESTING
mitkAddCustomModuleTest(mitkImageVtkMapper2D_rgbaImage640x480 mitkImageVtkMapper2DTest
                        ${MITK_DATA_DIR}/RenderingTestData/rgbaImage.png #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/rgbaImage640x480REF.png #corresponding reference screenshot
)
mitkAddCustomModuleTest(mitkImageVtkMapper2D_pic3d640x480 mitkImageVtkMapper2DTest #test for standard Pic3D axial slice
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/pic3d640x480REF.png #corresponding reference screenshot
)
mitkAddCustomModuleTest(mitkImageVtkMapper2D_pic3dColorBlue640x480 mitkImageVtkMapper2DColorTest #test for color property (=blue) Pic3D sagittal slice
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/pic3dColorBlue640x480REF.png #corresponding reference screenshot
)
mitkAddCustomModuleTest(mitkImageVtkMapper2D_pic3dLevelWindow640x480 mitkImageVtkMapper2DLevelWindowTest #test for levelwindow property (=blood) #Pic3D sagittal slice
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/pic3dLevelWindowBlood640x480REF.png #corresponding reference #screenshot
)
#mitkAddCustomModuleTest(mitkImageVtkMapper2D_pic3dOpacity640x480 mitkImageVtkMapper2DOpacityTest #test for opacity (=0.5) Pic3D coronal slice
#                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
#                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/pic3dOpacity640x480REF.png corresponding reference screenshot
#)
mitkAddCustomModuleTest(mitkImageVtkMapper2D_pic3dSwivel640x480 mitkImageVtkMapper2DSwivelTest #test for a randomly chosen Pic3D swivelled slice
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/pic3dSwivel640x480REF.png #corresponding reference screenshot
)
mitkAddCustomModuleTest(mitkPointSetVtkMapper2D_openMeAlone640x480 mitkPointSetVtkMapper2DTest
                        ${MITK_DATA_DIR}/RenderingTestData/openMeAlone.mps #input point set to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/openMeAlone640x480REF.png #corresponding reference screenshot
)
mitkAddCustomModuleTest(mitkPointSetVtkMapper2D_Pic3DPointSetForPic3D640x480 mitkPointSetVtkMapper2DImageTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd ${MITK_DATA_DIR}/RenderingTestData/PointSetForPic3D.mps #input point set and image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/Pic3DPointSetForPic3D640x480REF.png #corresponding reference screenshot
)
mitkAddCustomModuleTest(mitkPointSetVtkMapper2D_openMeAloneGlyphType640x480 mitkPointSetVtkMapper2DGlyphTypeTest
                         ${MITK_DATA_DIR}/RenderingTestData/openMeAlone.mps #input point set to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/openMeAloneGlyphType640x480REF.png #corresponding reference screenshot
)
mitkAddCustomModuleTest(mitkPointSetVtkMapper2D_openMeAloneTransformed640x480 mitkPointSetVtkMapper2DTransformedPointsTest
                         ${MITK_DATA_DIR}/RenderingTestData/openMeAlone.mps #input point set to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/openMeAloneTransformedPoints640x480REF.png #corresponding reference screenshot
)
mitkAddCustomModuleTest(mitkSurfaceDepthSortingTransparency_StanfordBunnySTL640x480 mitkSurfaceDepthSortingTest
                        ${MITK_DATA_DIR}/RenderingTestData/Stanford_bunny.stl
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/Stanford_bunnySTLDepthSorting640x480REF.png
)
if(NOT APPLE)
mitkAddCustomModuleTest(mitkSurfaceDepthPeelingTransparency_StanfordBunnySTL640x480 mitkSurfaceDepthPeelingTest
                        ${MITK_DATA_DIR}/RenderingTestData/Stanford_bunny.stl
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/Stanford_bunnySTLDepthPeeling640x480REF.png #corresponding reference screenshot
)
endif()

#Test reslice interpolation
#note: nearest mode is already tested by swivel test
mitkAddCustomModuleTest(ResliceInterpolationIsLinear mitkImageVtkMapper2DResliceInterpolationPropertyTest
                        1 #linear
                        ${MITK_DATA_DIR}/Pic3D.nrrd
                        -V
                        ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/pic3dRefLinear.png #corresponding reference screenshot LINEAR
)

mitkAddCustomModuleTest(ResliceInterpolationIsCubic mitkImageVtkMapper2DResliceInterpolationPropertyTest
                        3 #cubic
                        ${MITK_DATA_DIR}/Pic3D.nrrd
                        -V
                        ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/pic3dRefCubic.png #corresponding reference screenshot CUBIC
)
#End test reslice interpolation

# Testing of the rendering of binary images
#mitkAddCustomModuleTest(mitkImageVtkMapper2D_binaryTestImage640x480 mitkImageVtkMapper2DTest #test for standard Pic3D axial slice
#                        ${MITK_DATA_DIR}/RenderingTestData/binaryImage.nrrd #input image to load in data storage
#                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/binaryImage640x480REF.png #corresponding reference screenshot
#)
#mitkAddCustomModuleTest(mitkImageVtkMapper2D_binaryTestImageWithRef640x480 mitkImageVtkMapper2DTest #test for standard Pic3D axial slice
#                        ${MITK_DATA_DIR}/Pic3D.nrrd ${MITK_DATA_DIR}/RenderingTestData/binaryImage.nrrd #input image to load in data storage
#                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/binaryImageWithRef640x480REF.png #corresponding reference screenshot
#)
# End of binary image tests

mitkAddCustomModuleTest(mitkSurfaceVtkMapper3DTest_TextureProperty mitkSurfaceVtkMapper3DTest
                        ${MITK_DATA_DIR}/ToF-Data/Kinect_LiverPhantom.vtp
                        ${MITK_DATA_DIR}/ToF-Data/Kinect_LiverPhantom_RGBImage.nrrd
                        -V
                        ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/texturedLiver640x480REF.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkImageVtkMapper2DTransferFunctionTest_Png2D-bw mitkImageVtkMapper2DTransferFunctionTest
                        ${MITK_DATA_DIR}/Png2D-bw.png
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/Png2D-bw-TransferFunctionRGBImage640x480REF.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkImageVtkMapper2DOpacityTransferFunctionTest_Png2D-bw mitkImageVtkMapper2DOpacityTransferFunctionTest
                        ${MITK_DATA_DIR}/Png2D-bw.png
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/Png2D-bw-OpacityTransferFunctionRGBImage640x480REF.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkSurfaceGLMapper2DColorTest_RedBall mitkSurfaceGLMapper2DColorTest
                        ${MITK_DATA_DIR}/ball.stl
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/ballColorRed640x480REF.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkSurfaceGLMapper2DColorTest_DasArmeSchwein mitkSurfaceGLMapper2DColorTest
                        ${MITK_DATA_DIR}/binary.stl
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/binaryColorRed640x480REF.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkSurfaceGLMapper2DOpacityTest_BallOpacity mitkSurfaceGLMapper2DOpacityTest #opacity = 50% (0.5)
                        ${MITK_DATA_DIR}/ball.stl
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/ballOpacity640x480REF.png #corresponding reference screenshot
)

############################## DISABLED TESTS

#Removed due to high rendering error.
#mitkAddCustomModuleTest(mitkSurfaceVtkMapper3DTexturedSphereTest_Football mitkSurfaceVtkMapper3DTexturedSphereTest
#                        ${MITK_DATA_DIR}/RenderingTestData/texture.jpg #input texture
#                        -V
#                        ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/texturedSphere640x480REF.png corresponding reference screenshot
#)

mitkAddCustomModuleTest(mitkImageVtkMapper2DLookupTableTest_Png2D-bw mitkImageVtkMapper2DLookupTableTest
                        ${MITK_DATA_DIR}/Png2D-bw.png
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/Png2D-bw-LookupTableRGBImage640x480REF.png #corresponding reference screenshot
)

#mitkAddCustomModuleTest(mitkImageTest_color2DImage mitkImageTest
#                        ${MITK_DATA_DIR}/NrrdWritingTestImage.jpg
#)

#mitkAddCustomModuleTest(mitkNodeDependentPointSetInteractorTest mitkNodeDependentPointSetInteractorTest
#                        ${MITK_DATA_DIR}/Pic3D.pic.gz ${MITK_DATA_DIR}/BallBinary30x30x30.pic.gz
#)
SET_PROPERTY(TEST mitkImageVtkMapper2D_rgbaImage640x480 mitkImageVtkMapper2D_pic3d640x480 mitkImageVtkMapper2D_pic3dColorBlue640x480 mitkImageVtkMapper2D_pic3dLevelWindow640x480 mitkImageVtkMapper2D_pic3dSwivel640x480 mitkImageVtkMapper2DTransferFunctionTest_Png2D-bw
  # mitkImageVtkMapper2D_pic3dOpacity640x480
  mitkSurfaceGLMapper2DOpacityTest_BallOpacity mitkSurfaceGLMapper2DColorTest_DasArmeSchwein mitkSurfaceGLMapper2DColorTest_RedBall mitkSurfaceVtkMapper3DTest_TextureProperty mitkPointSetVtkMapper2D_Pic3DPointSetForPic3D640x480 mitkPointSetVtkMapper2D_openMeAlone640x480 mitkPointSetVtkMapper2D_openMeAloneGlyphType640x480 mitkPointSetVtkMapper2D_openMeAloneTransformed640x480 #mitkSurfaceVtkMapper3DTexturedSphereTest_Football
PROPERTY RUN_SERIAL TRUE)

endif()

add_test(mitkPointSetLocaleTest  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkPointSetLocaleTest ${MITK_DATA_DIR}/pointSet.mps)
set_property(TEST mitkPointSetLocaleTest PROPERTY LABELS MITK-Core)

add_test(mitkImageWriterTest_nrrdImage ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkImageWriterTest ${MITK_DATA_DIR}/NrrdWritingTestImage.jpg)
add_test(mitkImageWriterTest_2DPNGImage ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkImageWriterTest ${MITK_DATA_DIR}/Png2D-bw.png)
add_test(mitkImageWriterTest_rgbPNGImage ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkImageWriterTest ${MITK_DATA_DIR}/RenderingTestData/rgbImage.png)
add_test(mitkImageWriterTest_rgbaPNGImage ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkImageWriterTest ${MITK_DATA_DIR}/RenderingTestData/rgbaImage.png)
set_property(TEST mitkImageWriterTest_nrrdImage PROPERTY LABELS MITK-Core)
set_property(TEST mitkImageWriterTest_2DPNGImage PROPERTY LABELS MITK-Core)
set_property(TEST mitkImageWriterTest_rgbPNGImage PROPERTY LABELS MITK-Core)
set_property(TEST mitkImageWriterTest_rgbaPNGImage PROPERTY LABELS MITK-Core)

add_subdirectory(DCMTesting)
