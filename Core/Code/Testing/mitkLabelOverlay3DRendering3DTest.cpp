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


int mitkLabelOverlay3DRendering3DTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkLabelOverlay3DRendering3DTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

  renderingHelper.SetMapperIDToRender3D();
//  renderingHelper.SetAutomaticallyCloseRenderWindow(false);

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(renderingHelper.GetVtkRenderWindow());
  mitk::OverlayManager::Pointer OverlayManager = mitk::OverlayManager::New();
  renderer->SetOverlayManager(OverlayManager);

  mitk::PointSet::Pointer ps = mitk::PointSet::New();
  mitk::LabelOverlay3D::Pointer to3d = mitk::LabelOverlay3D::New();
  mitk::Point3D offset;
  offset[0] = .5;
  offset[1] = .5;
  offset[2] = .5;
  std::vector<std::string> labels;
  int idx = 0;
  for(int i=-10 ; i < 10 ; i+=2){
    for(int j=-10 ; j < 10 ; j+=2){
      mitk::Point3D point;
      point[0] = i;
      point[1] = j;
      point[2] = (i*j)/10-30;
      ps->InsertPoint(idx++, point);
      labels.push_back("test");
    }
  }

  to3d->SetLabelCoordinates(ps);
  to3d->SetLabelVector(labels);
  to3d->SetOffsetVector(offset);
  OverlayManager->AddOverlay(to3d.GetPointer());

  mitk::DataNode::Pointer dn = mitk::DataNode::New();
  dn->SetData(ps);
  dn->SetName("pointSet");
  renderingHelper.AddNodeToStorage(dn);

  renderingHelper.Render();

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  //use this to generate a reference screenshot or save the file:
  bool generateReferenceScreenshot = false;
  if(generateReferenceScreenshot)
  {
    renderingHelper.SaveReferenceScreenShot("/home/christoph/Pictures/RenderingTestData/mitkLabelOverlay3DRendering3DTest.png");
  }


  MITK_TEST_END();
}

