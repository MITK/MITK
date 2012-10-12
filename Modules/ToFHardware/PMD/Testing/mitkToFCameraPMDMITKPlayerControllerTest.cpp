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

}git st
