/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNDITrackingDevice.h"
#include "mitkNDIPassiveTool.h"
#include "mitkTestingMacros.h"
#include "mitkTrackingTypes.h"
#include "mitkTrackingTool.h"
#include "mitkStandardFileLocations.h"


int mitkNDIPolarisHardwareTest(int argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("NDIPolarisHardware (An NDI tracking device has to be connected to this system!)");

  if (argc<2) {MITK_TEST_FAILED_MSG(<<"Error: test must be called with the com port as second parameter!");}

  int comPort = *argv[1] - '0';

  // let's create an object of our class
  mitk::NDITrackingDevice::Pointer myNDITrackingDevice = mitk::NDITrackingDevice::New();

  //TODO: set port!

  //TODO: add tools

  //OpenConnection
  MITK_TEST_CONDITION( (myNDITrackingDevice->OpenConnection()), "Testing method OpenConnection().");

  //StartTracking
  MITK_TEST_CONDITION( (myNDITrackingDevice->StartTracking()), "Testing method StartTracking().");

  //TODO: test if tools are updated correctly

  //TODO: stop tracking

  //CloseConnection
  MITK_TEST_CONDITION( (myNDITrackingDevice->CloseConnection()), "Testing behavior of method CloseConnection().");

  // always end with this!
  MITK_TEST_END();
}