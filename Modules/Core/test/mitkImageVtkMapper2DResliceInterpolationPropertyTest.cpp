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

// MITK
#include "mitkDataNode.h"
#include "mitkIOUtil.h"
#include "mitkRenderingTestHelper.h"
#include "mitkTestingMacros.h"
#include <mitkNodePredicateDataType.h>

// VTK
#include <mitkVtkResliceInterpolationProperty.h>
#include <vtkRegressionTestImage.h>

int mitkImageVtkMapper2DResliceInterpolationPropertyTest(int argc, char *argv[])
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
  // load all arguments into a datastorage, take last argument as reference
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  // note: this test is supposed to test the reslice interpolation modes not the swiveling itself
  MITK_TEST_BEGIN("mitkImageVtkMapper2DResliceInterpolationPropertyTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

  // get resliceInterpolation from comandline arg
  int resliceInterpolation = atoi(argv[argc - 4]);

  // Set interpolation mode for rendering
  renderingHelper.SetImageProperty("reslice interpolation",
                                   mitk::VtkResliceInterpolationProperty::New(resliceInterpolation));

  /*+++rotate plane+++*/
  // center point for rotation
  mitk::Point3D centerPoint;
  centerPoint.Fill(0.0f);
  // vector for rotating the slice
  mitk::Vector3D rotationVector;
  rotationVector.SetElement(0, 0.2);
  rotationVector.SetElement(1, 0.3);
  rotationVector.SetElement(2, 0.5);
  // sets a swivel direction for the image

  // new version of setting the center point:
  mitk::Image::Pointer image = static_cast<mitk::Image *>(
    renderingHelper.GetDataStorage()->GetNode(mitk::NodePredicateDataType::New("Image"))->GetData());

  // get the center point of the image
  centerPoint = image->GetGeometry()->GetCenter();

  // rotate the image arround its own center
  renderingHelper.ReorientSlices(centerPoint, rotationVector);

  // threshold for CompareRenderWindowAgainstReference
  double threshold = 0.35; // difference between interpolation modes is very small

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION(renderingHelper.CompareRenderWindowAgainstReference(argc, argv, threshold) == true,
                      "CompareRenderWindowAgainstReference test result positive?");

  // use this to generate a reference screenshot or save the file:
  if (false)
  {
    renderingHelper.Render();
    renderingHelper.SaveReferenceScreenShot("C:\\Users\\schroedt\\Pictures\\RenderingTestData\\Pic3dRefLinear.png");
  }

  MITK_TEST_END();
}
