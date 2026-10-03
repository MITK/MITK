MITK_CREATE_MODULE_TESTS(DEPENDS MitkSceneSerializationBase)
if(TARGET ${TESTDRIVER})
  mitk_use_modules(TARGET ${TESTDRIVER} PACKAGES VTK|TestingRendering)

  # viewportTest():
  #
  # 1. Sets up a scene with a DICOM image, several planar figures, an STL file
  # 2. Modifies the viewport of the VTK renderer to only a part of the render window
  # 3. Compares the rendering result against an expected version
  #
  # The test postfix encodes the aspect ratio (Square, Landscape, Portrait) of the
  # render window (r), the viewport (v) and the input image (w).
  # The expected result is mitkViewportRenderingTest_${testPostfix}.png
  #
  function(viewportTest
    testPostfix  # postfix to uniquely identify the test case
    rwWidth rwHeight # "rw" = render window
    vpLeft vpBottom vpWidth vpHeight # "vp" = viewport
    worldAspect # aspect ratio of the input image
    )
    math(EXPR vpRight "${vpLeft}   + ${vpWidth}")
    math(EXPR vpTop   "${vpBottom} + ${vpHeight}")

    mitkAddCustomModuleRenderingTest(
        mitkViewportRenderingTest_${testPostfix}
        mitkViewportRenderingTest
        ${rwWidth} ${rwHeight}
        ${vpLeft} ${vpBottom}
        ${vpRight} ${vpTop}
        ${MITK_DATA_DIR}/RenderingTestData/viewport/input_${worldAspect}/SCSFREN.dcm # nice DICOM name encoding and gray value test image
        ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Line1.pf
        ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Path1.pf
        ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/FourPointAngle1.pf
        ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Rectangle1.pf
        ${MITK_DATA_DIR}/binary.stl
        -V ${MITK_DATA_DIR}/RenderingTestData/viewport/mitkViewportRenderingTest_${testPostfix}.png #corresponding reference screenshot
        )
  endfunction()

  # The camera is fit to the image before the viewport is set, so the fit depends
  # only on the image aspect relative to the render window. One case per fit
  # direction, each with a viewport of the opposite orientation, covers both fit
  # branches and catches mixed-up viewport axes in the planar figure mapper.
  viewportTest(rSvLwP 500 500 30 45 450 300 Portrait)  # fit limited by height
  viewportTest(rSvPwL 500 500 30 45 300 450 Landscape) # fit limited by width
endif() # endif TARGET ${TESTDRIVER}
