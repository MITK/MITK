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
#include <mitkLevelWindowProperty.h>
#include <mitkLevelWindowPreset.h>

//VTK
#include <vtkRegressionTestImage.h>

int mitkImageVtkMapper2DLevelWindowTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkImageVtkMapper2DTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);
  //chose a level window: here we randomly chosen the blood preset.
  mitk::LevelWindowPreset* levelWindowPreset = mitk::LevelWindowPreset::New();
  bool loadedPreset = levelWindowPreset->LoadPreset();
  MITK_TEST_CONDITION_REQUIRED(loadedPreset == true, "Testing if level window preset could be loaded");
  double level = levelWindowPreset->getLevel("Blood");
  double window = levelWindowPreset->getWindow("Blood");
  levelWindowPreset->Delete();
  //apply level window to all images
  renderingHelper.SetImageProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(level, window)) );
  //for now this test renders Sagittal
  renderingHelper.SetViewDirection(mitk::SliceNavigationController::Sagittal);

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  //use this to generate a reference screenshot or save the file:
  if(false)
  {
    renderingHelper.SaveReferenceScreenShot("/media/hdd/thomasHdd/Pictures/tmp/output1.png");
  }

  MITK_TEST_END();
}

