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
#include <mitkToFCameraPMDCamCubeController.h>

/**Documentation
 *  test for the class "ToFCameraPMDCamCubeController".
 */
int mitkToFCameraPMDCamCubeControllerTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("ToFCameraPMDCamCubeController");
  mitk::ToFCameraPMDCamCubeController::Pointer testObject = mitk::ToFCameraPMDCamCubeController::New();
  MITK_TEST_CONDITION_REQUIRED(!testObject.GetPointer()==NULL,"Testing initialzation!");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureHeight()== 0 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureWidth()== 0 ,"Testing initialization of CaptureWidth");
  MITK_TEST_CONDITION_REQUIRED(testObject->CloseCameraConnection(),"Testing closing of camera connection!");

  MITK_TEST_END();

}


