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
#include "mitkTextOverlay3D.h"
#include <mitkPointSet.h>


int mitkTextOverlay3DRendering2DTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkTextOverlay3DRendering2DTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(renderingHelper.GetVtkRenderWindow());
  mitk::OverlayManager::Pointer overlayManager = mitk::OverlayManager::New();
  renderer->SetOverlayManager(overlayManager);

  mitk::PointSet::Pointer pointset = mitk::PointSet::New();

  // This vector is used to define an offset for the annotations, in order to show them with a margin to the actual coordinate.
  mitk::Point3D offset;
  offset[0] = .5;
  offset[1] = .5;
  offset[2] = .5;

  //Just a loop to create some points
  for(int i=0 ; i < 5 ; i++){
      //To each point, a TextOverlay3D is created
      mitk::TextOverlay3D::Pointer textOverlay3D = mitk::TextOverlay3D::New();
      mitk::Point3D point;
      point[0] = i*2;
      point[1] = i*3;
      point[2] = -i*5;
      pointset->InsertPoint(i, point);
      textOverlay3D->SetText("A Point");

      // The Position is set to the point coordinate to create an annotation to the point in the PointSet.
      textOverlay3D->SetPosition3D(point);

      // move the annotation away from the actual point
      textOverlay3D->SetOffsetVector(offset);

      overlayManager->AddOverlay(textOverlay3D.GetPointer());
  }

  // also show the created pointset
  mitk::DataNode::Pointer datanode = mitk::DataNode::New();
  datanode->SetData(pointset);
  datanode->SetName("pointSet");
  renderingHelper.AddNodeToStorage(datanode);
  renderingHelper.Render();

  //use this to generate a reference screenshot or save the file:
  bool generateReferenceScreenshot = true;
  if(generateReferenceScreenshot)
  {
    renderingHelper.SaveReferenceScreenShot("d:/tmp/mitkTextOverlay3DRendering2DTest_ball.png");
  }

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  MITK_TEST_END();
}

