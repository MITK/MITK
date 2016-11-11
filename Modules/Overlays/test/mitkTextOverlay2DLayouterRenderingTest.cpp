/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// MITK
#include "mitkRenderingTestHelper.h"
#include "mitkTestingMacros.h"

// VTK
#include "mitkOverlayLayouter2D.h"
#include "mitkTextOverlay2D.h"
#include <vtkRegressionTestImage.h>

mitk::TextOverlay2D::Pointer createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Alignment align,
                                                           mitk::BaseRenderer *renderer,
                                                           int fontsize,
                                                           float red,
                                                           float green,
                                                           float blue,
                                                           int prio,
                                                           std::string text)
{
  // Create a textOverlay2D
  mitk::TextOverlay2D::Pointer textOverlay = mitk::TextOverlay2D::New();

  textOverlay->SetText(text);
  textOverlay->SetFontSize(fontsize);
  textOverlay->SetColor(red, green, blue);
  textOverlay->SetOpacity(1);

  mitk::OverlayLayouter2D::AddOverlay(textOverlay, renderer, align, 30, 30, prio);

  return textOverlay;
}

int mitkTextOverlay2DLayouterRenderingTest(int argc, char *argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkTextOverlay2DLayouterRenderingTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);
  //  renderingHelper.SetAutomaticallyCloseRenderWindow(false);

  mitk::BaseRenderer *renderer = mitk::BaseRenderer::GetInstance(renderingHelper.GetVtkRenderWindow());

  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::TopLeft, renderer, 20, 1.0, 1.0, 1.0, 1, "TopLeft1");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::TopLeft, renderer, 15, 1.0, 1.0, 1.0, 3, "TopLeft3");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::TopLeft, renderer, 25, 1.0, 0.0, 1.0, 2, "TopLeft2");

  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Top, renderer, 15, 1.0, 1.0, 1.0, 3, "Top3");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Top, renderer, 20, 1.0, 1.0, 1.0, 1, "Top1");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Top, renderer, 25, 1.0, 0.0, 1.0, 2, "Top2");

  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::TopRight, renderer, 20, 1.0, 1.0, 1.0, 1, "TopRight1");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::TopRight, renderer, 15, 1.0, 1.0, 1.0, 3, "TopRight3");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::TopRight, renderer, 25, 1.0, 0.0, 1.0, 2, "TopRight2");

  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Left, renderer, 20, 1.0, 1.0, 1.0, 1, "Left1");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Left, renderer, 15, 1.0, 1.0, 1.0, 3, "Left3");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Left, renderer, 25, 1.0, 0.0, 1.0, 2, "Left2");

  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Right, renderer, 25, 1.0, 0.0, 1.0, 2, "Right2");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Right, renderer, 20, 1.0, 1.0, 1.0, 1, "Right1");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Right, renderer, 15, 1.0, 1.0, 1.0, 3, "Right3");

  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::BottomLeft, renderer, 25, 1.0, 0.0, 1.0, 2, "BottomLeft2");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::BottomLeft, renderer, 20, 1.0, 1.0, 1.0, 1, "BottomLeft1");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::BottomLeft, renderer, 15, 1.0, 1.0, 1.0, 3, "BottomLeft3");

  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Bottom, renderer, 15, 1.0, 1.0, 1.0, 3, "Bottom3");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Bottom, renderer, 20, 1.0, 1.0, 1.0, 1, "Bottom1");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::Bottom, renderer, 25, 1.0, 0.0, 1.0, 2, "Bottom2");

  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::BottomRight, renderer, 25, 1.0, 0.0, 1.0, 2, "BottomRight2");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::BottomRight, renderer, 20, 1.0, 1.0, 1.0, 1, "BottomRight1");
  createTextOverlayWithLayouter(mitk::OverlayLayouter2D::BottomRight, renderer, 15, 1.0, 1.0, 1.0, 3, "BottomRight3");

  renderingHelper.Render();

  renderingHelper.SetAutomaticallyCloseRenderWindow(false);
  // use this to generate a reference screenshot or save the file:
  bool generateReferenceScreenshot = false;
  if (generateReferenceScreenshot)
  {
    renderingHelper.SaveReferenceScreenShot(
      "/home/christoph/Pictures/RenderingTestData/mitkTextOverlay2DLayouterRenderingTest_ball.png");
  }

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION(renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true,
                      "CompareRenderWindowAgainstReference test result positive?");

  MITK_TEST_END();
}
