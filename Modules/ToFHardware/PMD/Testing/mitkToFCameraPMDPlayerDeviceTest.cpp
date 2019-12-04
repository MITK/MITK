/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkToFCameraPMDPlayerDevice.h>

#include <mitkToFConfig.h>
#include <mitkToFPMDConfig.h>

/**Documentation
 *  test for the class "ToFCameraPMDPlayerDevice".
 */
int mitkToFCameraPMDPlayerDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDPlayerDevice");

  std::string fileName = MITK_TOF_DATA_DIR;
  fileName = fileName + "/TestSequence.pmd";
  mitk::ToFCameraPMDPlayerDevice::Pointer tofCameraPMDPlayerDevice = mitk::ToFCameraPMDPlayerDevice::New();
  tofCameraPMDPlayerDevice->SetProperty("PMDFileName",mitk::StringProperty::New(fileName));
  std::string platform = MITK_TOF_PLATFORM;
  // PMDPlayerController only available for W32 systems
  if (platform=="W32")
  {
    if(std::string(MITK_TOF_PMDFILE_SOURCE_PLUGIN) != "")
    {
      MITK_TEST_CONDITION_REQUIRED(tofCameraPMDPlayerDevice->ConnectCamera(),"Test ConnectCamera()");
    }
    tofCameraPMDPlayerDevice->StartCamera();
    tofCameraPMDPlayerDevice->UpdateCamera();
    float* distances = new float[40000];
    float* amplitudes = new float[40000];
    float* intensities = new float[40000];
    int imageSequence = 0;
    MITK_TEST_OUTPUT(<<"Call GetDistances()");
    tofCameraPMDPlayerDevice->GetDistances(distances,imageSequence);
    MITK_TEST_OUTPUT(<<"Call GetAmplitudes()");
    tofCameraPMDPlayerDevice->GetAmplitudes(amplitudes,imageSequence);
    MITK_TEST_OUTPUT(<<"Call GetIntensities()");
    tofCameraPMDPlayerDevice->GetIntensities(intensities,imageSequence);
    tofCameraPMDPlayerDevice->StopCamera();
    MITK_TEST_CONDITION_REQUIRED(tofCameraPMDPlayerDevice->DisconnectCamera(),"Test DisconnectCamera()");

    delete[] distances;
    delete[] amplitudes;
    delete[] intensities;
  }

  MITK_TEST_END();

}
