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
    MITK_TEST_CONDITION_REQUIRED((device->GetDeviceManufacturer().compare("Manufacturer") == 0), "Manufacturer should be set correctly");
    MITK_TEST_CONDITION_REQUIRED((device->GetDeviceModel().compare("Model") == 0), "Model should be set correctly");
  }

   static void TestAddProbe()
  {
    //mitk::USDevice::Pointer device = mitk::USVideoDevice::New("IllegalPath", "Manufacturer", "Model");
    mitk::USVideoDevice::Pointer  tempPointer =  mitk::USVideoDevice::New("IllegalPath", "Manufacturer", "Model");
    mitk::USDevice* device = tempPointer.GetPointer();

    // create probes
    mitk::USProbe::Pointer usSource = mitk::USProbe::New();
    mitk::USProbe::Pointer probeA = mitk::USProbe::New();
    mitk::USProbe::Pointer probeB = mitk::USProbe::New();
    mitk::USProbe::Pointer identicalProbe = mitk::USProbe::New(); // only this one should be identical

    // give my babys some names
    probeA->SetName("ProbeA");
    probeB->SetName("ProbeB");
    identicalProbe->SetName("ProbeA");
    // I'm gonna be a bad father...

    //right now, list of devices should be empty
    MITK_TEST_CONDITION_REQUIRED(device->GetConnectedProbes().size() == 0, "Newly created device should have no probes connected");

    // Connect Probe A
    device->AddProbe(probeA);
    MITK_TEST_CONDITION_REQUIRED(device->GetConnectedProbes().size() == 1, "Device should add one new probe");

    // Connect Probe B
    device->AddProbe(probeB);
    MITK_TEST_CONDITION_REQUIRED(device->GetConnectedProbes().size() == 2, "Device should add another probe");

    // Connect identical Probe
    device->AddProbe(identicalProbe);
    MITK_TEST_CONDITION_REQUIRED(device->GetConnectedProbes().size() == 2, "Device should not have added identical probe");
  }


  static void TestActivateProbe()
  {
    //mitk::USDevice::Pointer device = mitk::USVideoDevice::New("IllegalVideoDevice", "Manufacturer", "Model");
    mitk::USVideoDevice::Pointer  tempPointer =  mitk::USVideoDevice::New("IllegalPath", "Manufacturer", "Model");
    mitk::USDevice* device = tempPointer.GetPointer();
    // create probes
    mitk::USProbe::Pointer usSource = mitk::USProbe::New();
    mitk::USProbe::Pointer probeA = mitk::USProbe::New();
    mitk::USProbe::Pointer probeB = mitk::USProbe::New();
    mitk::USProbe::Pointer identicalProbe = mitk::USProbe::New(); // only this one should be identical

    // names
    probeA->SetName("ProbeA");
    probeB->SetName("ProbeB");
    identicalProbe->SetName("ProbeA");
    device->AddProbe(probeA);
    device->AddProbe(probeB);

    // We after activation, we expect the device to activate probeA, which is the first-connected identical version.
    device->ActivateProbe(identicalProbe);
    MITK_TEST_CONDITION_REQUIRED(device->GetActiveProbe() == probeA, "probe A should be active");

    // And we deactivate it again...
    device->DeactivateProbe();
    MITK_TEST_CONDITION_REQUIRED(device->GetActiveProbe().IsNull(), "After deactivation, no probe should be active");
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
