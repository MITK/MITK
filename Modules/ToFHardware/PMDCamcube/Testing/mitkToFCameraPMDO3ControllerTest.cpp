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
#include <mitkToFCameraPMDO3Controller.h>

/**Documentation
 *  test for the class "ToFCameraPMDO3Controller".
 */
int mitkToFCameraPMDO3ControllerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDO3Controller");
  mitk::ToFCameraPMDO3Controller::Pointer testObject = mitk::ToFCameraPMDO3Controller::New();
  MITK_TEST_CONDITION_REQUIRED(!testObject.GetPointer()==NULL,"Testing initialzation!");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureHeight()== 200 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureWidth()== 200 ,"Testing initialization of CaptureWidth");
  MITK_TEST_CONDITION_REQUIRED(testObject->OpenCameraConnection(),"Testing OpenCameraConnection!");
  MITK_TEST_CONDITION_REQUIRED(testObject->CloseCameraConnection(),"Testing closing of camera connection!");

  MITK_TEST_END();

}
