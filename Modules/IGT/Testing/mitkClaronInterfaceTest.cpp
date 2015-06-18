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

#include "mitkClaronInterface.h"

#include "mitkTestingMacros.h"

#include <iostream>

/**
 * This function is testing the Class ClaronInterface. For tests with this class, which connects directly to the device via firewire
 * by using the MTC-library we would need the MicronTracker hardware, so only instantiation is tested yet (2009, January, 23rd).
 * As soon as there is a working concept to test the tracking classes which are very close to the hardware on all systems more
 * tests are needed here.
 */
int mitkClaronInterfaceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ClaronInterface")

  // let's create an object of our class
  mitk::ClaronInterface::Pointer myClaronInterface = mitk::ClaronInterface::New();
  myClaronInterface->Initialize("Test","Test");

  // first test: did this work?
  MITK_TEST_CONDITION_REQUIRED((myClaronInterface.IsNotNull()),"Testing instantiation:")

  // if the Microntracker is not installed we could also test the returnValues of the stubs
  if (!myClaronInterface->IsMicronTrackerInstalled())
    {
    MITK_TEST_OUTPUT(<< "In the following stubs are tested, errors are expected and should occur.")
    MITK_TEST_CONDITION_REQUIRED((myClaronInterface->GetAllActiveTools().empty()),"Testing stub of GetAllActiveTools() ");
    MITK_TEST_CONDITION_REQUIRED((myClaronInterface->GetName(0)==nullptr),"Testing stub of GetName() ");
    MITK_TEST_CONDITION_REQUIRED((myClaronInterface->GetPosition(0).empty()),"Testing stub of GetPosition() ");
    MITK_TEST_CONDITION_REQUIRED((myClaronInterface->GetQuaternions(0).empty()),"Testing stub of GetQuaternions() ");
    MITK_TEST_CONDITION_REQUIRED((myClaronInterface->GetTipPosition(0).empty()),"Testing stub of GetTipPosition() ");
    MITK_TEST_CONDITION_REQUIRED((myClaronInterface->GetTipQuaternions(0).empty()),"Testing stub of GetTipQuaternions() ");
    MITK_TEST_CONDITION_REQUIRED(!(myClaronInterface->StartTracking()),"Testing stub of StartTracking() ");
    MITK_TEST_CONDITION_REQUIRED(!(myClaronInterface->StopTracking()),"Testing stub of StopTracking() ");
    bool success = true;
    try
      {
      myClaronInterface->GrabFrame();
      }
    catch(...)
      {
      success = false;
      }
    MITK_TEST_CONDITION_REQUIRED(success,"Testing stub of GrabFrame() ");
    }

  // always end with this!
  MITK_TEST_END()
}


