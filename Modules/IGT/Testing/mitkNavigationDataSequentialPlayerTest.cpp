/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 14:52:01 +0200 (Mi, 13. Mai 2009) $
Version:   $Revision: 17230 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkNavigationDataSequentialPlayer.h>
#include <mitkStandardFileLocations.h>
#include "mitkTestingMacros.h"

#include <iostream>
#include <sstream>

const char* XML_STRING = "<?xml version=\"1.0\" ?>\n<Version Ver=\"1\" />\n"
"<Data ToolCount=\"1\">\n"
  "<ND Time=\"3.990000\" Tool=\"0.000000\" X=\"1.000000\" Y=\"0.000000\" Z=\"3.000000\" QX=\"0.000000\" QY=\"0.000000\" QZ=\"0.000000\" QR=\"0.000000\" C00=\"1.000000\" C01=\"0.000000\" C02=\"0.000000\" C03=\"0.000000\" C04=\"0.000000\" C05=\"0.000000\" C10=\"0.000000\" C11=\"1.000000\" C12=\"0.000000\" C13=\"0.000000\" C14=\"0.000000\" C15=\"0.000000\" Valid=\"0\" hO=\"1\" hP=\"1\" />\n"
  "<ND Time=\"83.630000\" Tool=\"0.000000\" X=\"2.000000\" Y=\"1.000000\" Z=\"4.000000\" QX=\"0.000000\" QY=\"0.000000\" QZ=\"0.000000\" QR=\"0.000000\" C00=\"1.000000\" C01=\"0.000000\" C02=\"0.000000\" C03=\"0.000000\" C04=\"0.000000\" C05=\"0.000000\" C10=\"0.000000\" C11=\"1.000000\" C12=\"0.000000\" C13=\"0.000000\" C14=\"0.000000\" C15=\"0.000000\" Valid=\"0\" hO=\"1\" hP=\"1\" />\n"
"</Data>";

/**Documentation
 *  test for the class "NavigationDataRecorder".
 */
int mitkNavigationDataSequentialPlayerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataSequentialPlayer");

  mitk::NavigationDataSequentialPlayer::Pointer navigationDataPlayer
      = mitk::NavigationDataSequentialPlayer::New();

  navigationDataPlayer
      ->SetXMLString(XML_STRING);

  navigationDataPlayer
      ->SetRepeat(true);

  mitk::NavigationData::Pointer navData;
  mitk::Point3D position;
  for(int i=0; i<6;++i)
  {
    navigationDataPlayer->Update();
    navData = navigationDataPlayer->GetOutput();

    // test some values
    MITK_TEST_CONDITION_REQUIRED(navData.IsNotNull(), "navData.IsNotNull()");
    position = navData->GetPosition();

    if(i==0)
    {
      MITK_TEST_CONDITION(position[2] == 3, "position[2] == 3");
    }
    else if(i==1)
    {
      MITK_TEST_CONDITION(position[2] == 4, "position[2] == 4");
    }
    else if(i==2) // should be repeated
    {
      MITK_TEST_CONDITION(position[2] == 3, "position[2] == 3");
    }
    else if(i==3)
    {
      MITK_TEST_CONDITION(position[2] == 4, "position[2] == 4");
      navigationDataPlayer->SetRepeat(false); // remove repeat flag now
    }
    else if(i==4)
    {
      // repeat is removed, now all coming nd should be invalid
      MITK_TEST_CONDITION(navData->IsDataValid() == false, "Testing"
                          " for invalid data at end of recording data.");
    }

  }

  // always end with this!
  MITK_TEST_END();
}
