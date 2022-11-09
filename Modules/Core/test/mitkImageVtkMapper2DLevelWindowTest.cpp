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
#include <mitkLevelWindowPreset.h>
#include <mitkLevelWindowProperty.h>

// VTK
#include <vtkRegressionTestImage.h>

int mitkImageVtkMapper2DLevelWindowTest(int argc, char *argv[])
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
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkImageVtkMapper2DTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);
  // chose a level window: here we randomly chosen the blood preset.
  mitk::LevelWindowPreset *levelWindowPreset = mitk::LevelWindowPreset::New();
  bool loadedPreset = levelWindowPreset->LoadPreset();
  MITK_TEST_CONDITION_REQUIRED(loadedPreset == true, "Testing if level window preset could be loaded");
  double level = levelWindowPreset->getLevel("Blood");
  double window = levelWindowPreset->getWindow("Blood");
  levelWindowPreset->Delete();
  // apply level window to all images
  renderingHelper.SetImageProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(level, window)));
  // for now this test renders Sagittal
  renderingHelper.SetViewDirection(mitk::AnatomicalPlane::Sagittal);

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION(renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true,
                      "CompareRenderWindowAgainstReference test result positive?");

  // use this to generate a reference screenshot or save the file:
  if (false)
  {
    renderingHelper.SaveReferenceScreenShot("/media/hdd/thomasHdd/Pictures/tmp/output1.png");
  }

  MITK_TEST_END();
}
