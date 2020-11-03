/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkKinectV2Controller.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkPixelType.h>

/**Documentation
*  test for the class "KinectV2Controller".
*/
int mitkKinectV2ControllerTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("KinectV2ControllerTest");
  try
  {
    int counter = 0;


    mitk::KinectV2Controller::Pointer kinectController = mitk::KinectV2Controller::New();
    MITK_INFO << "connected: " << kinectController->OpenCameraConnection();
    MITK_INFO << "CloseCameraConnection: " << kinectController->CloseCameraConnection();
    MITK_INFO << "connected: " << kinectController->OpenCameraConnection();

    for( int i = 0; i < 10; ++i)
    {
      //try to work with a kinect controller
      unsigned int height = 424;
      unsigned int width = 512;
      float* depthArray = new float[height*width];
      unsigned char* rgbhArray = new unsigned char[1920*1080*3];

      if(!kinectController->UpdateCamera())
      {
        counter++;
        continue;
      }
      kinectController->GetDistances(depthArray);
      kinectController->GetRgb(rgbhArray);


      mitk::Image::Pointer image = mitk::Image::New();
      unsigned int dim[2];
      dim[0] = width;
      dim[1] = height;
      image->Initialize(mitk::PixelType(mitk::MakeScalarPixelType<float>()), 2, dim, 1);
      image->SetSlice(depthArray);

      mitk::Image::Pointer rgbimage = mitk::Image::New();
      unsigned int dimRGB[2];
      dimRGB[0] = 1920;
      dimRGB[1] = 1080;
      rgbimage->Initialize(mitk::PixelType(mitk::MakePixelType<unsigned char, itk::RGBPixel<unsigned char>, 3>()), 2, dimRGB, 1);
      rgbimage->SetSlice(rgbhArray);

      std::stringstream ss;
      ss << "C:/tom/test" << i << ".nrrd";
      mitk::IOUtil::Save(image, ss.str());

      std::stringstream ss2;
      ss2 << "C:/tom/rgb" << i << ".nrrd";
      mitk::IOUtil::Save(rgbimage, ss2.str());
    }
    MITK_INFO << "counter: " << counter;
  }
  catch(std::exception &e)
  {
    MITK_INFO << e.what();
  }

  MITK_TEST_END();

}
