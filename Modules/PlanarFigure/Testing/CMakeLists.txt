MITK_CREATE_MODULE_TESTS()

set(testcaseBasename mitkViewportRenderingTest)

# set verbose to 1 to get a list of all defined test cases during CMake configuration
set(verbose 0)

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
      -V ${MITK_DATA_DIR}/RenderingTestData/viewport/mitkViewportRenderingTest_${testPostfix}.png #corresponding reference screenshot
      )
  set_property(TEST mitkViewportRenderingTest_${testPostfix}
               PROPERTY RUN_SERIAL TRUE)
endfunction()

function(getFirstCharacter string charVar)
  string(SUBSTRING ${string} 0 1 c)
  set(${charVar} ${c} PARENT_SCOPE)
endfunction()


# The following lines loop over several variations of aspect ratios for
# - render window
# - a viewport within this render window
# - an input image (defines the input world geometry)
# ---------------------------------------------------------------------
#
# Render window
#  - Square
#  - Landscape
#  - Portrait
#
# Viewport
#  - Square
#  - Landscape
#  - Portrait
#
# World / Image
#  - Square
#  - Landscape
#  - Portrait
#
if(MITK_ENABLE_RENDERING_TESTING) ### since the rendering test's do not run in ubuntu, yet, we build them only for other systems or if the user explicitly sets the variable MITK_ENABLE_RENDERING_TESTING
foreach(renderWindowAspect Square Landscape Portrait)
  foreach(viewportAspect Square Landscape Portrait)
    foreach(worldAspect Square Landscape Portrait)
      getFirstCharacter(${renderWindowAspect} renderWindowAspectSymbol)
      getFirstCharacter(${viewportAspect} viewportAspectSymbol)
      getFirstCharacter(${worldAspect} worldAspectSymbol)

      # construct test case name from various aspect ratios
      set(testCaseShortname "r${renderWindowAspectSymbol}v${viewportAspectSymbol}w${worldAspectSymbol}")

      # construct test image name from aspect ratio
      set(testImage ${MITK_DATA_DIR}/RenderingTestData/viewport/input_${worldAspect}/SCSFREN.dcm) # nice DICOM name encoding and gray value test image
      set(testObjects
        ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Line1.pf
        ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Path1.pf
        ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/FourPointAngle1.pf
        ${MITK_DATA_DIR}/RenderingTestData/PlanarFigures/Rectangle1.pf
        ${MITK_DATA_DIR}/binary.stl
      )

      # render window size
      if (renderWindowAspect STREQUAL Landscape)
        set(renderWindowWidth 600)
      else()
        set(renderWindowWidth 500)
      endif()

      if (renderWindowAspect STREQUAL Portrait)
        set(renderWindowHeight 600)
      else()
        set(renderWindowHeight 500)
      endif()

      # viewport size
      if (viewportAspect STREQUAL Landscape)
        set(viewportWidth 450)
      else()
        set(viewportWidth 300)
      endif()

      if (viewportAspect STREQUAL Portrait)
        set(viewportHeight 450)
      else()
        set(viewportHeight 300)
      endif()

      # world size
      if (worldAspect STREQUAL Portrait)
        set(worldWidth 300)
        set(worldHeight 420)
      elseif (worldAspect STREQUAL Landscape)
        set(worldWidth 420)
        set(worldHeight 300)
      else()
        set(worldWidth 512)
        set(worldHeight 512)
      endif()

      # Summary
      set(viewportX 30)
      set(viewportY 45)
      if (verbose)
        message(STATUS "(${testCaseShortname}) "
                       "Render window ${renderWindowAspect} (${renderWindowWidth}x${renderWindowHeight}), "
                       "Viewport ${viewportAspect} (${viewportWidth}x${viewportHeight}+${viewportX}+${viewportY}), "
                       "World ${worldAspect} (${worldWidth}x${worldHeight})"
               )
      endif()

      viewPortTest(${testCaseShortname}
                   ${renderWindowWidth} ${renderWindowHeight}
                   ${viewportX} ${viewportY}
                   ${viewportWidth} ${viewportHeight}
                   ${testImage}
                   ${testObjects})

    endforeach()
  endforeach()
endforeach()
endif() # endif MITK_ENABLE_RENDERING_TESTING
