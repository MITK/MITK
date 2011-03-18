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


