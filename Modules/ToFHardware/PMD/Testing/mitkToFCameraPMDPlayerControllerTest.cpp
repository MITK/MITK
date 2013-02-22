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
#include "mitkToFConfig.h"
#include <mitkTestingMacros.h>
#include <mitkToFCameraPMDPlayerController.h>
#include <mitkToFCameraPMDPlayerDevice.h>

#include "mitkToFPMDConfig.h"
#include <mitkToFConfig.h>

/**Documentation
 *  test for the class "ToFCameraPMDPlayerController".
 */

int mitkToFCameraPMDPlayerControllerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDPlayerController");

  //initialize test
  mitk::ToFCameraPMDPlayerDevice::Pointer pmdPlayerDevice = mitk::ToFCameraPMDPlayerDevice::New();
  mitk::ToFCameraPMDPlayerController::Pointer pmdPlayerController = mitk::ToFCameraPMDPlayerController::New();

  MITK_TEST_CONDITION_REQUIRED( pmdPlayerController.IsNotNull(),"Testing object initialization!");
  MITK_TEST_CONDITION_REQUIRED( pmdPlayerController->GetCaptureHeight()== 200,"Testing GetCaptureHeight()");
  MITK_TEST_CONDITION_REQUIRED( pmdPlayerController->GetCaptureWidth() == 200 ,"Testing GetCaptureWidth()");

  //nice one!
  MITK_TEST_CONDITION_REQUIRED( !pmdPlayerController->OpenCameraConnection(),"Testing OpenConnection without valid data!");

  // set some valid data and test connecting again! TODO: Set the data to generic dir!!
  std::string filePath = MITK_TOF_DATA_DIR;
  std::string fileName = "/TestSequence.pmd";
  filePath.append(fileName);
  pmdPlayerDevice->SetStringProperty("PMDFileName", filePath.c_str());
  pmdPlayerController = dynamic_cast<mitk::ToFCameraPMDPlayerController*>(pmdPlayerDevice->GetController().GetPointer());
  // current implementation for mitkToFCameraPMDPlayerController only works for 32 bit machines, so we need to check
  // the platform we are working on!
  std::string  platformVar = MITK_TOF_PLATFORM;

  if( platformVar == "W32")
  {
    if(std::string(MITK_TOF_PMDFILE_SOURCE_PLUGIN) != "")
    {
      MITK_TEST_CONDITION_REQUIRED( pmdPlayerController->OpenCameraConnection(),"Testing OpenConnection with valid data!");
      MITK_TEST_CONDITION_REQUIRED( !pmdPlayerController->GetIntegrationTime() == 0, "Testing passing of integration time from PMD data!");
      MITK_TEST_CONDITION_REQUIRED( !pmdPlayerController->GetModulationFrequency() == 0, "Testing passing of modulation frequency from PMD data!")
    }
    // testing disconnection
    MITK_TEST_CONDITION_REQUIRED ( pmdPlayerController->CloseCameraConnection(), "Testing CloseConnection with data!");
  }
  else
  {
    MITK_TEST_CONDITION_REQUIRED( true, "Test case skipped! Wrong platfrom configuration, no driver available!");
  }
  // test end
  MITK_TEST_END();

}
