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
#include <mitkToFOpenCVImageGrabber.h>

#include <mitkImageDataItem.h>
#include <mitkIOUtil.h>
#include <mitkToFCameraMITKPlayerDevice.h>
#include <mitkToFConfig.h>
#include <mitkImagePixelReadAccessor.h>

static bool CompareImages(mitk::Image::Pointer mitkImage, cv::Mat openCVImage)
{
  float equal = true;
  if (static_cast<int>((mitkImage->GetDimension(0))!=openCVImage.cols)||(static_cast<int>(mitkImage->GetDimension(1))!=openCVImage.rows))
  {
    equal = false;
  }
  mitk::ImagePixelReadAccessor<float,2> imageAcces(mitkImage, mitkImage->GetSliceData(0));
  for(int i=0; i<openCVImage.cols; i++)
  {
    for(int j=0; j<openCVImage.rows; j++)
    {
      itk::Index<2> currentIndex;
      currentIndex[0] = i;
      currentIndex[1] = j;
      float mitkImageValue = imageAcces.GetPixelByIndex(currentIndex);
      float openCVImageValue = openCVImage.at<float>(j,i);
      if (!mitk::Equal(mitkImageValue,openCVImageValue))
      {
        equal = false;
      }
    }
  }
  return equal;
}

/**Documentation
 *  test for the class "ToFOpenCVImageGrabber".
 */
int mitkToFOpenCVImageGrabberTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFOpenCVImageGrabber");

  std::string dirName = MITK_TOF_DATA_DIR;
  mitk::ToFImageGrabber::Pointer tofImageGrabber = mitk::ToFImageGrabber::New();
  tofImageGrabber->SetCameraDevice(mitk::ToFCameraMITKPlayerDevice::New());
  std::string distanceFileName = dirName + "/PMDCamCube2_MF0_IT0_1Images_DistanceImage.pic";
  tofImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(distanceFileName));
  std::string amplitudeFileName = dirName + "/PMDCamCube2_MF0_IT0_1Images_AmplitudeImage.pic";
  tofImageGrabber->SetProperty("AmplitudeImageFileName",mitk::StringProperty::New(amplitudeFileName));
  std::string intensityFileName = dirName + "/PMDCamCube2_MF0_IT0_1Images_IntensityImage.pic";
  tofImageGrabber->SetProperty("IntensityImageFileName",mitk::StringProperty::New(intensityFileName));
  tofImageGrabber->Update();

  mitk::Image::Pointer image = mitk::IOUtil::LoadImage(distanceFileName);

  try
  {
    mitk::ToFOpenCVImageGrabber::Pointer tofOpenCVImageGrabber = mitk::ToFOpenCVImageGrabber::New();
    tofOpenCVImageGrabber->SetToFImageGrabber(tofImageGrabber);
    MITK_TEST_CONDITION_REQUIRED(tofImageGrabber==tofOpenCVImageGrabber->GetToFImageGrabber(),"Test Set/GetToFImageGrabber()");
    MITK_TEST_OUTPUT(<<"Call StartCapturing()");
    tofOpenCVImageGrabber->StartCapturing();
    cv::Mat cvImage = tofOpenCVImageGrabber->GetImage();
    MITK_TEST_CONDITION_REQUIRED(CompareImages(image,cvImage),"Test distance image");
    image = mitk::IOUtil::LoadImage(amplitudeFileName);
    tofOpenCVImageGrabber->SetImageType(1);
    cvImage = tofOpenCVImageGrabber->GetImage();
    MITK_TEST_CONDITION_REQUIRED(CompareImages(image,cvImage),"Test amplitude image");
    image = mitk::IOUtil::LoadImage(intensityFileName);
    tofOpenCVImageGrabber->SetImageType(2);
    cvImage = tofOpenCVImageGrabber->GetImage();
    MITK_TEST_CONDITION_REQUIRED(CompareImages(image,cvImage),"Test intensity image");
    MITK_TEST_OUTPUT(<<"Call StopCapturing()");
    tofOpenCVImageGrabber->StopCapturing();
  }
  catch(std::exception &e)
  {
    MITK_INFO << "Exception is: " << e.what();
  }

  MITK_TEST_END();

}


