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
#include <Overlays/mitkOverlayManager.h>

//VTK
#include <vtkRegressionTestImage.h>
#include <Overlays/mitkTextOverlay2D.h>
#include <Overlays/mitkOverlay2DLayouter.h>


int mitkTextOverlay3DRenderingTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkTextOverlay3DRenderingTest")

  mitkRenderingTestHelper renderingHelper(640, 480, argc, argv);

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(renderingHelper.GetVtkRenderWindow());
  mitk::OverlayManager::Pointer OverlayManagerCreated = mitk::OverlayManager::New();
  renderer->SetOverlayManager(OverlayManagerCreated);

  mitk::OverlayManager::Pointer OverlayManager = mitk::OverlayManager::GetServiceInstance();
  MITK_TEST_CONDITION_REQUIRED( OverlayManager.IsNotNull(), "Microservice created and retrievable" );
  MITK_TEST_CONDITION_REQUIRED( OverlayManager.GetPointer() == OverlayManagerCreated.GetPointer(), "Correct OverlayManager Microservice" );


  //This fetches an instance of the OverlayManager microservice
  mitk::OverlayManager::Pointer overlayManager = mitk::OverlayManager::GetServiceInstance();

  mitk::TextOverlay2D::Pointer textOverlay;

  //Create a textOverlay2D
  textOverlay = mitk::TextOverlay2D::New();

  textOverlay->SetText("Test!ä");

  //Position is committed as a Point2D Property
  mitk::Point2D pos;
  pos[0] = 10,pos[1] = 20;
  textOverlay->SetPosition2D(pos);

  //Add the overlay to the overlayManager. It is added to all registered renderers automaticly
  overlayManager->AddOverlay(textOverlay.GetPointer());

  //Set a Layouter to the overlay. If the Layouter is custom it has to be added to the OverlayManager first.
  overlayManager->SetLayouter(textOverlay.GetPointer(),mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM,renderer);

  renderingHelper.SetAutomaticallyCloseRenderWindow(false);

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

