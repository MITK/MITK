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

#include "mitkTrackingDeviceSourceConfigurator.h"
#include <mitkClaronTrackingDevice.h>
#include <mitkNavigationTool.h>
#include <mitkTestingMacros.h>

class mitkTrackingDeviceSourceConfiguratorTestClass
{
public:

  static void TestInstantiation()
  {
    // let's create an object of our class
    mitk::TrackingDeviceSourceConfigurator::Pointer testInstance;
    mitk::NavigationToolStorage::Pointer emptyStorage = mitk::NavigationToolStorage::New();
    mitk::TrackingDevice::Pointer dummyDevice = mitk::ClaronTrackingDevice::New();
    testInstance = mitk::TrackingDeviceSourceConfigurator::New(emptyStorage,dummyDevice);
    MITK_TEST_CONDITION_REQUIRED(testInstance.IsNotNull(),"Testing instantiation:");
  }

  static void TestSimpleClaronTrackingDevice()
  {
    mitk::TrackingDeviceSourceConfigurator::Pointer testInstance;

    mitk::NavigationToolStorage::Pointer claronStorage = mitk::NavigationToolStorage::New();

    mitk::NavigationTool::Pointer firstTool = mitk::NavigationTool::New();
    //firstTool

  }
};

int mitkTrackingDeviceSourceConfiguratorTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TrackingDeviceConfigurator");

  mitkTrackingDeviceSourceConfiguratorTestClass::TestInstantiation();
  
  MITK_TEST_END();
}