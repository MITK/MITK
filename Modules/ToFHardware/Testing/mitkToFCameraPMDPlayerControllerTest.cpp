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
#include "mitkToFConfig.h"
#include <mitkTestingMacros.h>
#include <mitkToFCameraPMDPlayerController.h>
#include <mitkToFCameraPMDPlayerDevice.h>

#include <mitkToFConfig.h>

/**Documentation
 *  test for the class "ToFCameraPMDPlayerController".
 */

int mitkToFCameraPMDPlayerControllerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDPlayerController");

  //initialize test
  mitk::ToFCameraPMDPlayerDevice::Pointer pmdPlayerDevice = mitk::ToFCameraPMDPlayerDevice::New(); 
  mitk::ToFCameraPMDPlayerController::Pointer testObject = mitk::ToFCameraPMDPlayerController::New();

  MITK_TEST_CONDITION_REQUIRED( !(testObject.GetPointer() == NULL),"Testing object initialization!");
  MITK_TEST_CONDITION_REQUIRED( testObject->GetCaptureHeight()== 200,"Testing member variable init 1!");
  MITK_TEST_CONDITION_REQUIRED( testObject->GetCaptureWidth() == 200 ,"Testing member variable init 2!");
  
  //testing the creation of a connection without valid data
  
  MITK_TEST_CONDITION_REQUIRED( !testObject->OpenCameraConnection(),"Testing OpenConnection without valid data!");
  MITK_TEST_CONDITION_REQUIRED( testObject->GetIntegrationTime() == 0, "Testing passing of integration time from PMD data!");
  MITK_TEST_CONDITION_REQUIRED( testObject->GetModulationFrequency() == 0, "Testing passing of modulation frequency from PMD data!");

  // set some valid data and test connecting again! TODO: Set the data to generic dir!!
  std::string filePath = MITK_TOF_DATA_DIR; 
  std::string fileName = "/TestSequence.pmd";
  filePath.append(fileName);
  pmdPlayerDevice->SetStringProperty("PMDFileName", filePath.c_str());
  testObject = dynamic_cast<mitk::ToFCameraPMDPlayerController*>(pmdPlayerDevice->GetController().GetPointer());
  // current implementation for mitkToFCameraPMDPlayerController only works for 32 bit machines, so we need to check 
  // the platform we are working on!
  std::string  platformVar = MITK_TOF_PLATFORM;

  if( platformVar == "W32")
  {
    if(std::string(MITK_TOF_PMDFILE_SOURCE_PLUGIN) != "")
    {
      MITK_TEST_CONDITION_REQUIRED( testObject->OpenCameraConnection(),"Testing OpenConnection with valid data!");
      MITK_TEST_CONDITION_REQUIRED( !testObject->GetIntegrationTime() == 0, "Testing passing of integration time from PMD data!");
      MITK_TEST_CONDITION_REQUIRED( !testObject->GetModulationFrequency() == 0, "Testing passing of modulation frequency from PMD data!")
    }
    // testing disconnection 
    MITK_TEST_CONDITION_REQUIRED ( testObject->CloseCameraConnection(), "Testing CloseConnection with data!");
  }
  else
  {
    MITK_TEST_CONDITION_REQUIRED( true, "Test case skipped! Wrong platfrom configuration, no driver available!");
  }
  // test end
  MITK_TEST_END();

}


