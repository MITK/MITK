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
#include <mitkToFCameraPMDCamCubeDevice.h>

/**Documentation
 *  test for the class "ToFCameraPMDCamCubeDevice".
 */
int mitkToFCameraPMDCamCubeDeviceTest(int /* argc */, char* /*argv*/[])
{
    MITK_TEST_BEGIN("ToFCameraPMDCamCubeDevice");

    mitk::ToFCameraPMDCamCubeDevice::Pointer camCubeDevice = mitk::ToFCameraPMDCamCubeDevice::New();
    // No hardware attached for automatic testing -> test correct error handling
    try
    {
        MITK_TEST_CONDITION_REQUIRED(camCubeDevice.IsNotNull(), "Testing initialization");
        MITK_TEST_CONDITION_REQUIRED(camCubeDevice->ConnectCamera(), "Test ConnectCamera()");
        MITK_TEST_CONDITION_REQUIRED(!camCubeDevice->IsCameraActive(), "Test IsCameraActive() before StartCamera()");
        MITK_TEST_OUTPUT(<<"Call StartCamera()");
        camCubeDevice->StartCamera();
        MITK_TEST_CONDITION_REQUIRED(camCubeDevice->IsCameraActive(), "Test IsCameraActive() after StartCamera()");
        MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
        camCubeDevice->UpdateCamera();
        int numberOfPixels = camCubeDevice->GetCaptureWidth()*camCubeDevice->GetCaptureHeight();
        MITK_INFO<<numberOfPixels;
        float* distances = new float[numberOfPixels];
        float* amplitudes = new float[numberOfPixels];
        float* intensities = new float[numberOfPixels];
        char* sourceData = new char[numberOfPixels];
        int requiredImageSequence = 0;
        int imageSequence = 0;
        camCubeDevice->GetDistances(distances,imageSequence);
        camCubeDevice->GetAmplitudes(amplitudes,imageSequence);
        camCubeDevice->GetIntensities(intensities,imageSequence);
        camCubeDevice->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence);
        MITK_TEST_CONDITION_REQUIRED(camCubeDevice->IsCameraActive(), "Test IsCameraActive() before StopCamera()");
        MITK_TEST_OUTPUT(<<"Call StopCamera()");
        camCubeDevice->StopCamera();
        MITK_TEST_CONDITION_REQUIRED(!camCubeDevice->IsCameraActive(), "Test IsCameraActive() after StopCamera()");

        MITK_TEST_CONDITION_REQUIRED(camCubeDevice->DisconnectCamera(), "Test DisonnectCamera()");
        delete[] distances;
        delete[] amplitudes;
        delete[] intensities;
        delete[] sourceData;
    }
    catch(std::exception &e)
    {
        MITK_INFO << e.what();
        MITK_TEST_CONDITION_REQUIRED(camCubeDevice->IsCameraActive()==false, "Testing that no connection could be established.");
    }

    MITK_TEST_END();
}
