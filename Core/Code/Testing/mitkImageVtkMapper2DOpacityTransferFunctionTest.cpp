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
#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunction.h>
#include <mitkRenderingModeProperty.h>


int mitkImageVtkMapper2DOpacityTransferFunctionTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkImageVtkMapper2DopacityTransferFunctionTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

  //define some controlpoints for a scalaropacity
  mitk::TransferFunction::ControlPoints scalarOpacityPoints;
  scalarOpacityPoints.push_back( std::make_pair( 255, 0 ) );
  scalarOpacityPoints.push_back( std::make_pair( 0, 0.1 ) );
  scalarOpacityPoints.push_back( std::make_pair( 127, 0.3 ) );

  //define an arbitrary colortransferfunction
  vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  colorTransferFunction->SetColorSpaceToRGB();
  colorTransferFunction->AddRGBPoint(0.0, 1, 0, 0); //black = red
  colorTransferFunction->AddRGBPoint(127.5, 0, 1, 0); //grey = green
  colorTransferFunction->AddRGBPoint(255.0, 0, 0, 1); //white = blue

  mitk::TransferFunction::Pointer transferFunction = mitk::TransferFunction::New();
  transferFunction->SetColorTransferFunction( colorTransferFunction );
  transferFunction->SetScalarOpacityPoints(scalarOpacityPoints);

  //set the rendering mode to use the transfer function
  renderingHelper.SetImageProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR));
  //set the property for the image
  renderingHelper.SetImageProperty("Image Rendering.Transfer Function", mitk::TransferFunctionProperty::New(transferFunction));

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv, 20.0) == true, "CompareRenderWindowAgainstReference test result positive?" );

  //renderingHelper.SetAutomaticallyCloseRenderWindow(false);

  //use this to generate a reference screenshot or save the file:
  if(false)
  {
    renderingHelper.SaveReferenceScreenShot("/tmp/output3.png");
  }

  MITK_TEST_END();
}
