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

#include <mitkTestingMacros.h>
#include <mitkKinect2Controller.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkPixelType.h>

/**Documentation
*  test for the class "Kinect2Controller".
*/
int mitkKinect2ControllerTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("Kinect2ControllerTest");
  try
  {
    mitk::Kinect2Controller::Pointer kinectController = mitk::Kinect2Controller::New();
    MITK_INFO << "connected: " << kinectController->OpenCameraConnection();

    //try to work with a kinect controller
    unsigned int height = 424;
    unsigned int width = 512;
    float* depthArray = new float[height*width];

    //MITK_INFO << "update: " << kinectController->UpdateCamera();
    kinectController->GetDistances(depthArray);

    mitk::Image::Pointer image = mitk::Image::New();
    unsigned int dim[2];
    dim[0] = width;
    dim[1] = height;
    image->Initialize(mitk::PixelType(mitk::MakeScalarPixelType<float>()), 2, dim, 1);
    image->SetSlice(depthArray);

    mitk::IOUtil::SaveImage(image, "C:/tom/test.nrrd");

  }
  catch(std::exception &e)
  {
    MITK_INFO << e.what();
  }

  MITK_TEST_END();

}
