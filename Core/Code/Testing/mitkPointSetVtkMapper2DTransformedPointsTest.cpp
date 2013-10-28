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
#include <mitkNodePredicateDataType.h>
#include <mitkEnumerationProperty.h>
#include <mitkBaseProperty.h>
#include <mitkPointSet.h>

//VTK
#include <vtkRegressionTestImage.h>


int mitkPointSetVtkMapper2DTransformedPointsTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkPointSetVtkMapper2DTransformedPointsTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

  renderingHelper.SetViewDirection(mitk::SliceNavigationController::Sagittal);

  mitk::DataNode* dataNode = renderingHelper.GetDataStorage()->GetNode(mitk::NodePredicateDataType::New("PointSet"));

  if(dataNode)
  {
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*> ( dataNode->GetData() );

    if(pointSet)
    {
    mitk::Point3D origin = pointSet->GetGeometry()->GetOrigin();

    origin[1] += 10;
    origin[2] += 15;

    pointSet->GetGeometry()->SetOrigin(origin);
    pointSet->Modified();
    dataNode->Update();
    }
  }

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  //use this to generate a reference screenshot or save the file:
  if(false)
  {
    renderingHelper.SaveReferenceScreenShot("D:/test/output.png");
  }

  MITK_TEST_END();
}