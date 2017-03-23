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

#include "mitkPolhemusInterface.h"
#include "mitkTestingMacros.h"
#include "mitkStandardFileLocations.h"
#include <mitkIGTConfig.h>

class mitkPolhemusTrackingDeviceHardwareTestClass
{
public:
};

/** This function is testing the Class PolhemusTrackingDevice in interaction with the hardware.
  * So a MicronTracker Tracking System has to be installed and connected to run this test.
  * The test needs the filenames of three toolfiles as arguments.
  */
int mitkPolhemusTrackingDeviceHardwareTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("PolhemusTrackingDeviceHardware");

  MITK_TEST_END();
}
