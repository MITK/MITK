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

#include "vector"
#include "mitkTrackingTypes.h"
#include "mitkTestingMacros.h"

/**Documentation
* ClaronTool has a protected constructor and a protected itkNewMacro
* so that only it's friend class ClaronTrackingDevice is able to instantiate
* tool objects. Therefore, we derive from ClaronTool and add a 
* public itkNewMacro, so that we can instantiate and test the class
*/

/**
 * This function tests the ClaronTool class.
 */
int mitkTrackingTypesTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TrackingTypes");


  std:: vector<mitk::TrackingDeviceData> ResultSet = mitk::GetDeviceDataForLine(mitk::TrackingSystemInvalid);
  MITK_TEST_CONDITION(ResultSet.size() == 1,"Test correct retrieval of DeviceData: Number of results");
  MITK_TEST_CONDITION(ResultSet[0].Line == mitk::TrackingSystemInvalid,"Test correct retrieval of DeviceData: Correct device");

  mitk::TrackingDeviceData data = mitk::GetFirstCompatibleDeviceDataForLine(mitk::TrackingSystemInvalid);
  MITK_TEST_CONDITION(data.Line == mitk::TrackingSystemInvalid,"Test correct retrieval of first compatible model");
  MITK_TEST_END();
}


