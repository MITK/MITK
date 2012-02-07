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
#include <mitkKinectDevice.h>

/**Documentation
 *  test for the class "KinectDevice".
 */
int mitkKinectDeviceTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("KinectDevice");

  mitk::KinectDevice::Pointer testObject = mitk::KinectDevice::New();

  MITK_TEST_CONDITION_REQUIRED(!testObject.GetPointer()==NULL,"Testing initialzation!");
  MITK_TEST_CONDITION_REQUIRED(testObject->ConnectCamera(),"Testing ConnectCamera()");
  MITK_TEST_OUTPUT(<<"Testing StartCamera()");
  testObject->StartCamera();
  int captureHeight = testObject->GetCaptureHeight();
  int captureWidth = testObject->GetCaptureWidth();
  MITK_TEST_CONDITION_REQUIRED(captureHeight== 480 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(captureWidth== 640 ,"Testing initialization of CaptureWidth");
  //
  MITK_TEST_OUTPUT(<<"Testing StopCamera()");
  testObject->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(testObject->DisconnectCamera(),"Testing DisconnectCamera()");
  
  MITK_TEST_END();

}
