/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-12 14:05:50 +0100 (Fr, 12 Mrz 2010) $
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkTestingMacros.h>
#include <mitkToFCameraMITKPlayerController.h>
#include <mitkToFCameraMITKPlayerDevice.h>
#include <mitkToFImageGrabber.h>

#include <mitkPixelType.h>

#include <mitkImageWriter.h>

namespace mitk
{
  class ToFCameraMITKPlayerDeviceImpl : public ToFCameraMITKPlayerDevice
  {

  public:
    inline ToFCameraMITKPlayerController::Pointer GetController(){return m_Controller;};

    inline ToFCameraMITKPlayerDeviceImpl(){};
    inline ~ToFCameraMITKPlayerDeviceImpl(){};
  private:
  };
} // end namespace mitk

/**Documentation
 *  test for the class "ToFCameraMITKPlayerController".
 */
void ReadFileFormatTestCase(std::string extension, mitk::ToFImageGrabber::Pointer grabber)
{
  MITK_INFO<<"Checking image processing with file format " << extension.c_str();
  //create some test image
  unsigned int* dim = new unsigned int[3];
  dim[0] = 100;
  dim[1] = 100;
  dim[2] = 1;
  int end = dim[0]*dim[1]*dim[2];
  float* data = new float[end];
  int index = 0;

  while(index != end)
  {
    data[index] = (float) index;
    index++;
  }

  mitk::Image::Pointer testImage = mitk::Image::New();

  mitk::PixelType FloatType = mitk::MakeScalarPixelType<float>();
  testImage->Initialize( FloatType, 3, dim);
  testImage->SetVolume(data);

  // save image as dist, ampl and inten image to file. 
  std::string distanceImageFileName("distTestImage");
  std::string amplitudeImageFileName("amplTestImage");
  std::string intensityImageFileName("intenTestImage");

  mitk::ImageWriter::Pointer writer = mitk::ImageWriter::New();
  writer->SetInput(testImage);
  writer->SetExtension(extension);
  writer->SetFileName(distanceImageFileName);
  writer->Update();

  writer->SetFileName(amplitudeImageFileName);
  writer->Update();

  writer->SetFileName(intensityImageFileName);
  writer->Update();

  // load the files from directory 

  grabber->SetStringProperty("DistanceImageFileName", (distanceImageFileName.append(extension)).c_str());
  grabber->SetStringProperty("AmplitudeImageFileName", (amplitudeImageFileName.append(extension)).c_str());
  grabber->SetStringProperty("IntensityImageFileName", (intensityImageFileName.append(extension)).c_str());

  MITK_TEST_CONDITION_REQUIRED(grabber->ConnectCamera() ,"Are the image files loaded correctly?");

  mitk::ToFCameraMITKPlayerController::Pointer testObject = static_cast<mitk::ToFCameraMITKPlayerDeviceImpl*>(grabber->GetCameraDevice())->GetController();
  // test load image from image grabber
  testObject->UpdateCamera();
  index = 0;
  bool distOK = true;
  bool amplOK = true;
  bool intenOK = true;
  float* dist = new float[end];
  float* ampl = new float[end];
  float* inten = new float[end];
  testObject->GetDistances( dist );
  testObject->GetAmplitudes( ampl );
  testObject->GetIntensities( inten );

  while( index != end )
  {
    if( dist[index] != data[index])
    {
      distOK = false;
    }
    if( ampl[index] != data[index])
    {
      amplOK = false;
    }
    if( inten[index] != data[index])
    {
      intenOK = false;
    }
    index++;
  }

  MITK_TEST_CONDITION_REQUIRED(distOK ,"Testing correct loading and passing of distance image information!");
  MITK_TEST_CONDITION_REQUIRED(amplOK ,"Testing correct loading and passing of amplitude image information!");
  MITK_TEST_CONDITION_REQUIRED(intenOK ,"Testing correct loading and passing of intensity image information!");
  MITK_TEST_CONDITION_REQUIRED(testObject->CloseCameraConnection(),"Testing disconnection of controller");

  //clean up and delete saved image files
  if(remove(distanceImageFileName.c_str())!=0)
  {
    MITK_ERROR<<"File: "<<distanceImageFileName<<" not successfully deleted!";
  }

  if(std::remove(amplitudeImageFileName.c_str())!=0)
  {
    MITK_ERROR<<"File: "<<amplitudeImageFileName<<" not successfully deleted!";
  }
  if(remove(intensityImageFileName.c_str())!=0)
  {
    MITK_ERROR<<"File: "<<intensityImageFileName<<" not successfully deleted!";
  }
  delete[] dist;
  delete[] ampl;
  delete[] inten;
  delete[] dim;
}

int mitkToFCameraMITKPlayerControllerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraMITKPlayerController");

  mitk::ToFCameraMITKPlayerController::Pointer testObject = mitk::ToFCameraMITKPlayerController::New();
  mitk::ToFCameraMITKPlayerDeviceImpl* deviceImpl = new mitk::ToFCameraMITKPlayerDeviceImpl;
  mitk::ToFImageGrabber::Pointer imageGrabber = mitk::ToFImageGrabber::New();
  imageGrabber->SetCameraDevice(deviceImpl);

  MITK_TEST_CONDITION_REQUIRED(!(testObject.GetPointer() == NULL) ,"Testing initialization class");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureHeight()== 0 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureWidth()== 0 ,"Testing initialization of CaptureWidth");

  //test correct order of file name member
  testObject->SetDistanceImageFileName("distTest");
  testObject->SetAmplitudeImageFileName("amplTest");
  testObject->SetIntensityImageFileName("intenTest");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetDistanceImageFileName()== "distTest" ,"Testing correct filename passing - Dist");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetAmplitudeImageFileName()== "amplTest" ,"Testing correct filename passing - Ampl");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetIntensityImageFileName()== "intenTest" ,"Testing correct filename passing - Inten");

  //test passing of file name from image grabber
  imageGrabber->SetStringProperty("DistanceImageFileName", "distPropertyTestName");
  imageGrabber->SetStringProperty("AmplitudeImageFileName", "amplPropertyTestName");
  imageGrabber->SetStringProperty("IntensityImageFileName", "intenPropertyTestName");
  testObject = deviceImpl->GetController();
  MITK_TEST_CONDITION_REQUIRED(testObject->GetDistanceImageFileName()== "distPropertyTestName" ,"Testing correct filename passing from Image Grabber- Dist");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetAmplitudeImageFileName()== "amplPropertyTestName" ,"Testing correct filename passing from Image Grabber- Ampl");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetIntensityImageFileName()== "intenPropertyTestName" ,"Testing correct filename passing from Image Grabber- Inten");

  // PIC Format is deprecated
  // ReadFileFormatTestCase(".pic", imageGrabber);
  ReadFileFormatTestCase(".nrrd", imageGrabber);

  deviceImpl->Delete();
  // end of test
  MITK_TEST_END();

}


