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

#include "mitkClaronTrackingDevice.h"
#include "mitkClaronTool.h"
#include "mitkTestingMacros.h"
#include "mitkStandardFileLocations.h"

class mitkClaronTrackingDeviceHardwareTestClass
{
public:

/** This function is testing the Class ClaronTrackingDevice in interaction with the hardware. So a MicronTracker Tracking System has to be installed and connected to run this test. */
int mitkClaronTrackingHardwareDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ClaronTrackingDeviceHardware");
  //TODO: test hardware here
  MITK_TEST_END();
}