/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkToFCameraPMDMITKPlayerDevice.h>

/**Documentation
 *  test for the class "ToFCameraPMDMITKPlayerDevice".
 */
int mitkToFCameraPMDMITKPlayerDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDMITKPlayerDevice");

  mitk::ToFCameraPMDMITKPlayerDevice::Pointer playerDevice = mitk::ToFCameraPMDMITKPlayerDevice::New();
  MITK_TEST_OUTPUT(<<"Test ConnectCamera()");
  playerDevice->ConnectCamera();
  MITK_TEST_OUTPUT(<<"Test StartCamera()");
  playerDevice->StartCamera();
  MITK_TEST_OUTPUT(<<"Test StopCamera()");
  playerDevice->StopCamera();
  MITK_TEST_OUTPUT(<<"Test DisconnectCamera()");
  playerDevice->DisconnectCamera();

  MITK_TEST_END();
}
