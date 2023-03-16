/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//MITK
#include "mitkTestingMacros.h"
#include "mitkRenderingTestHelper.h"
#include "mitkToFCameraMITKPlayerDevice.h"

//VTK
#include <vtkRegressionTestImage.h>

#include <mitkToFConfig.h>
#include <mitkPixelType.h>
#include <itksys/SystemTools.hxx>


int mitkPlayerLoadAndRenderRGBDataTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkPlayerLoadAndRenderRGBDataTest");

  try
  {
    mitk::ToFCameraMITKPlayerDevice::Pointer playerDevice = mitk::ToFCameraMITKPlayerDevice::New();

    MITK_TEST_CONDITION_REQUIRED(argc >=2, "Testing if enough input parameters are set. Usage: Testname, ImageName (must be in MITK_TOF_DATA_DIR), -V /path/to/reference/screenshot");
    std::string dirname = MITK_TOF_DATA_DIR;
    std::string rgbFileName = dirname + "/" + argv[1];
    playerDevice->SetProperty("RGBImageFileName",mitk::StringProperty::New(rgbFileName));

    MITK_TEST_CONDITION_REQUIRED(playerDevice->IsCameraActive()==false,"The device (player) should not be active before starting.");
    MITK_TEST_CONDITION_REQUIRED(playerDevice->ConnectCamera()==true,"ConnectCamera() should return true in case of success.");
    MITK_TEST_OUTPUT(<< "Device connected");
    playerDevice->StartCamera();
    MITK_TEST_OUTPUT(<< "Device started");
    MITK_TEST_CONDITION_REQUIRED(playerDevice->IsCameraActive()==true,"After starting the device, the device should be active.");

    //initialize an array with the correct size
    unsigned int captureWidth = playerDevice->GetCaptureWidth();
    unsigned int captureHeight = playerDevice->GetCaptureHeight();
    unsigned int numberOfPixels = captureWidth*captureHeight;
    unsigned char* rgbDataArray = new unsigned char[numberOfPixels*3];
    int imageSequence = 0;

    //fill the array with the device output
    playerDevice->GetRgb(rgbDataArray, imageSequence);

    //initialize an image and fill it with the array
    unsigned int dimension[2];
    dimension[0] = captureWidth;
    dimension[1] = captureHeight;
    mitk::Image::Pointer rgbImage = mitk::Image::New();
    rgbImage->Initialize(mitk::PixelType(mitk::MakePixelType<unsigned char, itk::RGBPixel<unsigned char>, 3>()), 2, dimension,1);
    rgbImage->SetSlice(rgbDataArray);

    //create a node to pass it to the mitkRenderingTestHelper
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(rgbImage);

    // load all arguments into a datastorage, take last argument as reference rendering
    // setup a renderwindow of fixed size X*Y
    // render the datastorage
    // compare rendering to reference image
    mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.Render();

    //use this to generate a reference screenshot or save the file:
    bool generateReferenceScreenshot = false;
    if(generateReferenceScreenshot)
    {
      renderingHelper.SaveReferenceScreenShot("/home/kilgus/Pictures/output.png");
    }

    //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
    MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

    //Wait some time to avoid threading issues.
    itksys::SystemTools::Delay(1000);
    playerDevice->StopCamera();
    MITK_TEST_OUTPUT(<< "Device stopped");
    MITK_TEST_CONDITION_REQUIRED(playerDevice->IsCameraActive()==false,"After stopping the device, the device should be inactive.");
    MITK_TEST_CONDITION_REQUIRED(playerDevice->DisconnectCamera()==true, "DisconnectCamera() should return true in case of success.");
    MITK_TEST_OUTPUT(<< "Device disconnected");
    delete[] rgbDataArray;
  }
  catch(std::exception  &e)
  {
    MITK_ERROR << "Unknown exception occured: " << e.what();
  }

  MITK_TEST_END();
}
