MITK_CREATE_MODULE_TESTS()

#
# mitkViewportRenderingTest_ImageSurfacePlanarFigure:
#
# 1. Sets up a scene with a DICOM image, several planar figures, on STL file
# 2. Modifies the viewport of the VTK renderer to only a part of the render window
# 3. Compares the rendering result against an expected version
#
mitkAddCustomModuleTest(
  mitkViewportRenderingTest_ImageSurfacePlanarFigure 
  mitkViewportRenderingTest
  500 500 # width height
  20 50   # left bottom
  420 450   # right top
  ${MITK_DATA_DIR}/RenderingTestData/SCSFREN.dcm # nice DICOM name encoding and gray value test image
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Line1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Path1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/FourPointAngle1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Rectangle1.pf
  ${MITK_DATA_DIR}/binary.stl
  -V 
  ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkViewportRenderingTest_ImageSurfacePlanarFigure.png #corresponding reference screenshot
)
