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
#include <mitkToFCameraPMDMITKPlayerController.h>
#include <mitkToFConfig.h>

/**Documentation
 *  test for the class "ToFCameraPMDMITKPlayerController". 
 */
int mitkToFCameraPMDMITKPlayerControllerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDMITKPlayerController");

  mitk::ToFCameraPMDMITKPlayerController::Pointer testObject = mitk::ToFCameraPMDMITKPlayerController::New();
  MITK_TEST_CONDITION_REQUIRED(!(testObject.GetPointer() == NULL) ,"Testing initialization class");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureHeight()== 200 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureWidth()== 200 ,"Testing initialization of CaptureWidth");
  // test empty file behavior
  std::string testFile = "";
  testObject->SetInputFileName(testFile);
  MITK_TEST_CONDITION_REQUIRED(testObject->OpenCameraConnection(),"Testing open camera without valid data");
  //MITK_TEST_CONDITION_REQUIRED(!testObject->CloseCameraConnection(),"Testing closing of connection");

  //test valid data behavior
  //CAVE: The following test cases are valid once rawData recording is fixed. Currently the 
  //      functionality is not working therefor the test cases are excluded!!

  //std::string filePath = MITK_TOF_DATA_DIR;
  //testFile = "/aa_raw.pic";
  //filePath.append(testFile);
  //testObject->SetInputFileName(filePath);
  //MITK_TEST_CONDITION_REQUIRED( testObject->OpenCameraConnection(),"Testing open camera with valid data");
  //MITK_TEST_CONDITION_REQUIRED( testObject->GetIntegrationTime() == 2500,"Testing correct setting of integration time!");
  //MITK_TEST_CONDITION_REQUIRED( testObject->GetModulationFrequency() == 20,"Testing correct setting of modulation frequency!");
  //
  ////test source data passing and updating!
  //int size = testObject->GetSourceDataStructSize();
  //MITK_TEST_CONDITION_REQUIRED( size != 0 , "Testing correct setting of source data size!" )
  //char* sourceData = NULL;
  //testObject->GetSourceData( sourceData );
  //MITK_TEST_CONDITION_REQUIRED( sourceData == NULL, "Testing setting of source data without update camera!");
  //testObject->UpdateCamera();
  //sourceData = new char[size];
  //testObject->GetSourceData(sourceData);
  //MITK_TEST_CONDITION_REQUIRED( sourceData != NULL, "Testing setting of source data with update camera!");
  //delete[] sourceData;
  MITK_TEST_CONDITION( testObject->CloseCameraConnection(),"Closing Connection!");

  MITK_TEST_END();

}


