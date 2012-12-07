MITK_CREATE_MODULE_TESTS()

set(testcaseBasename mitkViewportRenderingTest)

# viewportTest():
#
# 1. Sets up a scene with a DICOM image, several planar figures, an STL file
# 2. Modifies the viewport of the VTK renderer to only a part of the render window
# 3. Compares the rendering result against an expected version
#
# Parameters determine the size and position of the viewport and the render window
# The expected result is mitkViewportRenderingTest_${testPostfix}.png
#
function(viewportTest
  testPostfix  # postfix to uniquely identify the test case
  rwWidth rwHeight # "rw" = render window
  vpLeft vpBottom vpWidth vpHeight # "vp" = viewport
  # list of images/files for visualization
  )
  set(data ${ARGN}) # use all ADDITIONAL parameters after vpHeight as filenames

  # transform the input parameters in required test parameters
  math(EXPR vpRight "${vpLeft}   + ${vpWidth}")
  math(EXPR vpTop   "${vpBottom} + ${vpHeight}")

  #message(STATUS "Testing viewport of ${vpWidth}x${vpHeight} at ${vpLeft},${vpBottom} in render window of size ${rwWidth}x${rwHeight} ('${testPostfix}')")

  # add the actual test
  mitkAddCustomModuleTest(
      mitkViewportRenderingTest_${testPostfix}
      mitkViewportRenderingTest
      ${rwWidth} ${rwHeight}
      ${vpLeft} ${vpBottom}
      ${vpRight} ${vpTop}
      ${data}
      -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/mitkViewportRenderingTest_${testPostfix}.png #corresponding reference screenshot
      )
endfunction()


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
# Base set of tests on a quadratic image
#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

set(grayValuesTestImageAndFigures
  ${MITK_DATA_DIR}/RenderingTestData/dcm_quad/SCSFREN.dcm # nice DICOM name encoding and gray value test image
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Line1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Path1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/FourPointAngle1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Rectangle1.pf
  ${MITK_DATA_DIR}/binary.stl
)

# base test
viewPortTest(Base
  500 500  # render window size
  20 50    # viewport position
  400 400  # viewport size
  ${grayValuesTestImageAndFigures}
)

# high viewport test
#viewPortTest(BaseHighViewport
#  500 500  # render window size
#  10 50    # viewport position
#  420 490  # viewport size
#  ${grayValuesTestImageAndFigures}
#)

# wide viewport test
viewPortTest(BaseWideViewport
  500 500  # render window size
  20 50    # viewport position
  440 410  # viewport size
  ${grayValuesTestImageAndFigures}
)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
# Set of tests on an image that is more high than wide
#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


# different set of test data, same measurements, but image the is not quadratic
set(grayValuesTestImageAndFigures
  ${MITK_DATA_DIR}/RenderingTestData/dcm_high/SCSFREN_high.dcm
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Line1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Path1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/FourPointAngle1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Rectangle1.pf
  ${MITK_DATA_DIR}/binary.stl
)

# wide image, quad viewport test
viewPortTest(highImageQuadViewport
  500 500  # render window size
  10 20    # viewport position
  400 400  # viewport size
  ${grayValuesTestImageAndFigures}
)

# high image, high viewport test
viewPortTest(highImageHighViewport
  500 500  # render window size
  10 20    # viewport position
  250 400  # viewport size
  ${grayValuesTestImageAndFigures}
)

# high image, high viewport test
viewPortTest(highImagehighViewport
  500 500  # render window size
  10 20    # viewport position
  400 250  # viewport size
  ${grayValuesTestImageAndFigures}
)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
# Set of tests on an image that is more WIDE than high
# ALSO some tests in a RENDER WINDOW that is not quadratic
#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


# different set of test data, same measurements, but image the is not quadratic
set(grayValuesTestImageAndFigures
  ${MITK_DATA_DIR}/RenderingTestData/dcm_wide/SCSFREN_wide.dcm
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Line1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Path1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/FourPointAngle1.pf
  ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Rectangle1.pf
  ${MITK_DATA_DIR}/binary.stl
)

# wide image, quad viewport test
viewPortTest(wideImageQuadViewport
  500 500  # render window size
  10 20    # viewport position
  400 400  # viewport size
  ${grayValuesTestImageAndFigures}
)

# high image, high viewport test
viewPortTest(wideImageHighViewport
  500 500  # render window size
  10 20    # viewport position
  250 400  # viewport size
  ${grayValuesTestImageAndFigures}
)

# high image, high viewport test
viewPortTest(wideImagehighViewport
  500 500  # render window size
  10 20    # viewport position
  400 250  # viewport size
  ${grayValuesTestImageAndFigures}
)


# non-quadratic render windows

# wide image, wide render window, quad viewport test
viewPortTest(wideImageWideRWQuadViewport
  500 300  # render window size
  10 20    # viewport position
  250 250  # viewport size
  ${grayValuesTestImageAndFigures}
)

# wide image, high render window, quad viewport test
#viewPortTest(wideImageHighRWQuadViewport
#  300 500  # render window size
#  10 20    # viewport position
#  250 250  # viewport size
#  ${grayValuesTestImageAndFigures}
#)

