MITK_CREATE_MODULE_TESTS()

mitkAddCustomModuleTest(mitkOverlayRendering_2dOverlayImage mitkOverlayRenderingTest #OverlayTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/pic3dSwivel640x480REF.png #corresponding reference screenshot
)

# if(BUILD_TESTING AND MODULE_IS_ENABLED)
# mitkAddCustomModuleTest(mitkImageToOpenCVImageFilterTest mitkImageToOpenCVImageFilterTest
#                         ${MITK_DATA_DIR}/RenderingTestData/rgbImage.png
# )
# endif()
