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
#include <mitkBaseProperty.h>
#include <mitkSurface.h>

//VTK
#include <vtkRegressionTestImage.h>

int mitkSurfaceDepthSortingTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkRenderingDepthSortingTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

  renderingHelper.SetMapperIDToRender3D();

  mitk::DataNode* dataNode = renderingHelper.GetDataStorage()->GetNode(mitk::NodePredicateDataType::New("Surface"));

  if(dataNode)
   {
      dataNode->SetOpacity(0.8);
      dataNode->SetBoolProperty("Depth Sorting", true);
      dataNode->Update();
   }

    //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
    MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  MITK_TEST_END();
}