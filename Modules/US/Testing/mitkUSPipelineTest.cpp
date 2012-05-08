/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkUSVideoDevice.h"
#include "mitkTestingMacros.h"


class mitkUSPipelineTestClass
{
public:

  // Anm: Implementierung der einzelnen Testmethoden

  static void TestInstantiation()
  {
   // Set up a pipeline
    mitk::USVideoDevice::Pointer videoDevice = mitk::USVideoDevice::New("C:\\Users\\maerz\\Videos\\Debut\\us.avi", "Manufacturer", "Model");
    MITK_TEST_CONDITION_REQUIRED(videoDevice.IsNotNull(), "videoDevice should not be null after instantiation");
    videoDevice->GenerateData();
    mitk::USImage::Pointer result = videoDevice->GetOutput(0);
    MITK_TEST_CONDITION_REQUIRED(result.IsNotNull(), "resulting images should not be null");
    MITK_TEST_CONDITION_REQUIRED(result->GetMetadata()->GetDeviceModel().compare("Model") == 0   , "resulting images should have their metadata Set correctly");

  }



 

};

/**
* This function is setting up a pipeline and checks the output for validity.
*/
int mitkUSPipelineTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkUSPipelineTest");

    mitkUSPipelineTestClass::TestInstantiation();
    

  MITK_TEST_END();
}