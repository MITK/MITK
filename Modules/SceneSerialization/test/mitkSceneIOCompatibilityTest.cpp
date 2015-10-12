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

//VTK
#include <vtkRegressionTestImage.h>


int mitkSceneIOCompatibilityTest(int argc, char* argv[])
{
  // Load all arguments but last into a DataStorage,
  // Take last argument as reference rendering.
  //
  // Setup a renderwindow of fixed size X*Y.
  // Render the DataStorage.
  // Compare rendering to reference image.
  MITK_TEST_BEGIN("mitkSceneIOCompatibilityTest")

  mitk::RenderingTestHelper renderingHelper(200, 200, argc, argv);

  // Usage of CompareRenderWindowAgainstReference: See documentation of mitkRenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true,
                       "CompareRenderWindowAgainstReference test result positive?" );

  // You can use this to generate a new reference
  // on your platform or after changes:
  if(false)
  {
    renderingHelper.SaveReferenceScreenShot("/where/you/want.png");
  }

  MITK_TEST_END();
}

