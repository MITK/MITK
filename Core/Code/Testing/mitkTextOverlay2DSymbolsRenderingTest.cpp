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

//MITK
#include "mitkTestingMacros.h"
#include "mitkRenderingTestHelper.h"
#include <mitkOverlayManager.h>

//VTK
#include <vtkRegressionTestImage.h>
#include "mitkTextOverlay2D.h"
#include "mitkOverlay2DLayouter.h"

mitk::TextOverlay2D::Pointer createTextOverlaySymbols(int fontsize, float red, float green, float blue, int posX, int posY, std::string text)
{
  //Create a textOverlay2D
  mitk::TextOverlay2D::Pointer textOverlay = mitk::TextOverlay2D::New();

  textOverlay->SetText(text);
  textOverlay->SetFontSize(fontsize);
  textOverlay->SetColor(red,green,blue);
  textOverlay->SetOpacity(1);

  //Position is committed as a Point2D Property
  mitk::Point2D pos;
  pos[0] = posX,pos[1] = posY;
  textOverlay->SetPosition2D(pos);
  return textOverlay;
}

int mitkTextOverlay2DSymbolsRenderingTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkTextOverlay2DSymbolsRenderingTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);
  renderingHelper.SetAutomaticallyCloseRenderWindow(false);

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(renderingHelper.GetVtkRenderWindow());
  mitk::OverlayManager::Pointer OverlayManager = mitk::OverlayManager::New();
  renderer->SetOverlayManager(OverlayManager);

  //Add the overlay to the overlayManager. It is added to all registered renderers automaticly
  OverlayManager->AddOverlay(createTextOverlaySymbols(30,1,0,0,100,400,"ä ö ü ß Ö Ä Ü").GetPointer());
  OverlayManager->AddOverlay(createTextOverlaySymbols(30,0,1,0,400,400,"Ç æ Œ Æ").GetPointer());
  OverlayManager->AddOverlay(createTextOverlaySymbols(30,0,0,1,400,200,"¼ ₧ ø £ Ø").GetPointer());
  OverlayManager->AddOverlay(createTextOverlaySymbols(30,1,0,0,100,200,"Δ ξ Ψ Ω").GetPointer());

  renderingHelper.Render();

  //use this to generate a reference screenshot or save the file:
  bool generateReferenceScreenshot = false;
  if(generateReferenceScreenshot)
  {
    renderingHelper.SaveReferenceScreenShot("/home/christoph/Pictures/RenderingTestData/output.png");
  }

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  MITK_TEST_END();
}

