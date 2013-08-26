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

void createTextOverlayWithLayouter(mitk::OverlayManager::Pointer OverlayManager, const std::string& layouter, mitk::BaseRenderer* renderer,
                                   int fontsize, float red, float green, float blue, std::string text)
{
  //Create a textOverlay2D
  mitk::TextOverlay2D::Pointer textOverlay = mitk::TextOverlay2D::New();

  textOverlay->SetText(text);
  textOverlay->SetFontSize(fontsize);
  textOverlay->SetColor(red,green,blue);
  textOverlay->SetOpacity(1);

  //Position is committed as a Point2D Property
  mitk::Point2D pos;
  pos[0] = 0,pos[1] = 0;
  textOverlay->SetPosition2D(pos);
  OverlayManager->AddOverlay(textOverlay.GetPointer());
  OverlayManager->SetLayouter(textOverlay.GetPointer(),layouter,renderer);
}

int mitkTextOverlay2DLayouterRenderingTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkTextOverlay2DLayouterRenderingTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);
//  renderingHelper.SetAutomaticallyCloseRenderWindow(false);

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(renderingHelper.GetVtkRenderWindow());
  mitk::OverlayManager::Pointer OverlayManager = mitk::OverlayManager::New();
  renderer->SetOverlayManager(OverlayManager);

  OverlayManager->AddLayouter(mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_TOP(),renderer).GetPointer());
  OverlayManager->AddLayouter(mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT(),renderer).GetPointer());
  OverlayManager->AddLayouter(mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_TOPRIGHT(),renderer).GetPointer());
  OverlayManager->AddLayouter(mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM(),renderer).GetPointer());
  OverlayManager->AddLayouter(mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMLEFT(),renderer).GetPointer());
  OverlayManager->AddLayouter(mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMRIGHT(),renderer).GetPointer());

  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_TOP(),renderer,15,1,1,1,"TOP_1");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_TOP(),renderer,15,1,1,1,"TOP_2");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_TOP(),renderer,20,1,1,1,"TOP_3");

  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT(),renderer,15,1,1,1,"TOPLEFT_1");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT(),renderer,20,1,1,1,"TOPLEFT_2");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT(),renderer,15,1,1,1,"TOPLEFT_3");

  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_TOPRIGHT(),renderer,20,1,1,1,"TOPRIGHT_1");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_TOPRIGHT(),renderer,20,1,1,1,"TOPRIGHT_2");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_TOPRIGHT(),renderer,15,1,1,1,"TOPRIGHT_3");

  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM(),renderer,15,1,1,1,"BOTTOM_1");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM(),renderer,15,1,1,1,"BOTTOM_2");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM(),renderer,20,1,1,1,"BOTTOM_3");

  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMLEFT(),renderer,15,1,1,1,"BOTTOMLEFT_1");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMLEFT(),renderer,20,1,1,1,"BOTTOMLEFT_2");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMLEFT(),renderer,15,1,1,1,"BOTTOMLEFT_3");

  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMRIGHT(),renderer,20,1,1,1,"BOTTOMRIGHT_1");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMRIGHT(),renderer,20,1,1,1,"BOTTOMRIGHT_2");
  createTextOverlayWithLayouter(OverlayManager,mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMRIGHT(),renderer,15,1,1,1,"BOTTOMRIGHT_3");



  renderingHelper.Render();

  //use this to generate a reference screenshot or save the file:
  bool generateReferenceScreenshot = false;
  if(generateReferenceScreenshot)
  {
    renderingHelper.SaveReferenceScreenShot("/home/christoph/Pictures/RenderingTestData/mitkTextOverlay2DLayouterRenderingTest_ball.png");
  }

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  MITK_TEST_END();
}

