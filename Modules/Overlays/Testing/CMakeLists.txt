MITK_CREATE_MODULE_TESTS()

if(WIN32 OR APPLE OR MITK_ENABLE_GUI_TESTING)
mitkAddCustomModuleTest(mitkLabelOverlay3DRendering2DTest mitkLabelOverlay3DRendering2DTest #OverlayTest
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkLabelOverlay3DRendering2DTest.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkLabelOverlay3DRendering3DTest mitkLabelOverlay3DRendering3DTest #OverlayTest
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkLabelOverlay3DRendering3DTest.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkTextOverlay2DRenderingTest_ball mitkTextOverlay2DRenderingTest #OverlayTest
                        ${MITK_DATA_DIR}/ball.stl #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay2DRenderingTest_ball.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkTextOverlay2DLayouterRenderingTest_ball mitkTextOverlay2DLayouterRenderingTest #OverlayTest
                        ${MITK_DATA_DIR}/ball.stl #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay2DLayouterRenderingTest_ball.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkTextOverlay3DRendering2DTest_ball mitkTextOverlay3DRendering2DTest #OverlayTest
                        ${MITK_DATA_DIR}/ball.stl #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay3DRendering2DTest_ball.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkTextOverlay3DRendering3DTest_ball mitkTextOverlay3DRendering3DTest #OverlayTest
                        ${MITK_DATA_DIR}/ball.stl #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay3DRendering3DTest_ball.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkTextOverlay3DColorRenderingTest_ball mitkTextOverlay3DColorRenderingTest #OverlayTest
                        ${MITK_DATA_DIR}/ball.stl #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay3DColorRenderingTest_ball.png #corresponding reference screenshot
)

#pic3d

mitkAddCustomModuleTest(mitkTextOverlay2DRenderingTest_Pic3D mitkTextOverlay2DRenderingTest #OverlayTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay2DRenderingTest_Pic3D.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkTextOverlay2DLayouterRenderingTest_Pic3D mitkTextOverlay2DLayouterRenderingTest #OverlayTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay2DLayouterRenderingTest_Pic3D.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkTextOverlay3DRendering2DTest_Pic3D mitkTextOverlay3DRendering2DTest #OverlayTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay3DRendering2DTest_Pic3D.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkTextOverlay3DRendering3DTest_Pic3D mitkTextOverlay3DRendering3DTest #OverlayTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay3DRendering3DTest_Pic3D.png #corresponding reference screenshot
)

mitkAddCustomModuleTest(mitkTextOverlay3DColorRenderingTest_Pic3D mitkTextOverlay3DColorRenderingTest #OverlayTest
                        ${MITK_DATA_DIR}/Pic3D.nrrd #input image to load in data storage
                        -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkTextOverlay3DColorRenderingTest_Pic3D.png #corresponding reference screenshot
)
endif()
