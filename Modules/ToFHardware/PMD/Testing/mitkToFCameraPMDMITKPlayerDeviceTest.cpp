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
