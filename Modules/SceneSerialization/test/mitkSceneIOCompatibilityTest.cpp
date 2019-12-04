/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkRenderingTestHelper.h"
#include "mitkTestingMacros.h"

// VTK
#include <vtkRegressionTestImage.h>

int mitkSceneIOCompatibilityTest(int argc, char *argv[])
{
  try
  {
    mitk::RenderingTestHelper openGlTest(640, 480);
  }
  catch (const mitk::TestNotRunException &e)
  {
    MITK_WARN << "Test not run: " << e.GetDescription();
    return 77;
  }
  // Load all arguments but last into a DataStorage,
  // Take last argument as reference rendering.
  //
  // Setup a renderwindow of fixed size X*Y.
  // Render the DataStorage.
  // Compare rendering to reference image.
  MITK_TEST_BEGIN("mitkSceneIOCompatibilityTest")

  mitk::RenderingTestHelper renderingHelper(200, 200, argc, argv);

  // Usage of CompareRenderWindowAgainstReference: See documentation of mitkRenderingTestHelper
  MITK_TEST_CONDITION(renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true,
                      "CompareRenderWindowAgainstReference test result positive?");

  // You can use this to generate a new reference
  // on your platform or after changes:
  if (false)
  {
    renderingHelper.SaveReferenceScreenShot("/where/you/want.png");
  }

  MITK_TEST_END();
}
