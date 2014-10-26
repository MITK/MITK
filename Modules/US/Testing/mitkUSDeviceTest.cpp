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

#include "mitkUSVideoDevice.h"
#include "mitkUSProbe.h"
#include "mitkTestingMacros.h"

class mitkUSDeviceTestClass
{
public:

  static void TestInstantiation()
  {
    // let's create an object of our class
    mitk::USVideoDevice::Pointer  tempPointer =  mitk::USVideoDevice::New("IllegalPath", "Manufacturer", "Model");
    mitk::USDevice* device = tempPointer.GetPointer();
    MITK_TEST_CONDITION_REQUIRED(device, "USDevice should not be null after instantiation");
    //MITK_TEST_CONDITION_REQUIRED((device->GetDeviceManufacturer().compare("Manufacturer") == 0), "Manufacturer should be set correctly");
    //MITK_TEST_CONDITION_REQUIRED((device->GetDeviceModel().compare("Model") == 0), "Model should be set correctly");
  }

  static void TestAddProbe()
  {
  }

  static void TestActivateProbe()
  {
  }
};

/**
* This function is testing methods of the class USDevice.
*/
int mitkUSDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkUSDeviceTest");

  mitkUSDeviceTestClass::TestInstantiation();
  mitkUSDeviceTestClass::TestAddProbe();
  mitkUSDeviceTestClass::TestActivateProbe();

  MITK_TEST_END();
}