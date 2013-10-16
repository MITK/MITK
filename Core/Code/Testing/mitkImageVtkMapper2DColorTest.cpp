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


int mitkImageVtkMapper2DColorTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkImageVtkMapper2DTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);
  //Set the opacity for all images
  renderingHelper.SetImageProperty("color", mitk::ColorProperty::New(0.0f, 0.0f, 255.0f));
  //for now this test renders in sagittal view direction
  renderingHelper.SetViewDirection(mitk::SliceNavigationController::Sagittal);

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  //####################
  //Use this to generate a reference screenshot or save the file.
  //(Only in your local version of the test!)
  if(false)
  {
    renderingHelper.SaveReferenceScreenShot("/home/kilgus/Pictures/RenderingTestData/output.png");
  }
  //####################


  MITK_TEST_END();
}

