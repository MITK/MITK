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
#include <mitkPointSet.h>

//VTK
#include <vtkRegressionTestImage.h>
#include "mitkTextOverlay2D.h"
#include "mitkOverlay2DLayouter.h"
#include "mitkLabelOverlay3D.h"


int mitkLabelOverlay3DRendering2DTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkLabelOverlay3DRendering2DTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

//  renderingHelper.SetAutomaticallyCloseRenderWindow(false);
  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(renderingHelper.GetVtkRenderWindow());
  mitk::OverlayManager::Pointer OverlayManager = mitk::OverlayManager::New();
  renderer->SetOverlayManager(OverlayManager);

  mitk::PointSet::Pointer pointset = mitk::PointSet::New();
  mitk::LabelOverlay3D::Pointer label3d = mitk::LabelOverlay3D::New();
  mitk::Point3D offset;
  offset[0] = .5;
  offset[1] = .5;
  offset[2] = .5;

  std::vector<std::string> labels;
  int idx = 0;
  for(int i=-10 ; i < 10 ; i+=4){
    for(int j=-10 ; j < 10 ; j+=4){
      mitk::Point3D point;
      point[0] = i;
      point[1] = j;
      point[2] = (i*j)/10;
      pointset->InsertPoint(idx++, point);
      labels.push_back("test");
    }
  }

  label3d->SetLabelCoordinates(pointset);
  label3d->SetLabelVector(labels);
  label3d->SetOffsetVector(offset);
  OverlayManager->AddOverlay(label3d.GetPointer());

  mitk::DataNode::Pointer datanode = mitk::DataNode::New();
  datanode->SetData(pointset);
  datanode->SetName("pointSet");
  renderingHelper.AddNodeToStorage(datanode);
  renderingHelper.Render();

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  //use this to generate a reference screenshot or save the file:
  bool generateReferenceScreenshot = false;
  if(generateReferenceScreenshot)
  {
    renderingHelper.SaveReferenceScreenShot("/home/christoph/Pictures/RenderingTestData/mitkLabelOverlay3DRendering2DTest.png");
  }

  MITK_TEST_END();
}

