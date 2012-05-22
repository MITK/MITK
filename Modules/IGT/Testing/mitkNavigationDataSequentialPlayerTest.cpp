
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

#include <mitkNavigationDataSequentialPlayer.h>
#include <mitkStandardFileLocations.h>
#include "mitkTestingMacros.h"

#include <iostream>
#include <sstream>

const char* XML_STRING =
  "<?xml version=\"1.0\" ?><Version Ver=\"1\" /><Data ToolCount=\"2\">"
    "<NavigationData Time=\"1375.79\" Tool=\"0\" X=\"-279.14\" Y=\"40.48\" Z=\"-2023.72\" QX=\"0.0085\" QY=\"-0.0576\" QZ=\"-0.0022\" QR=\"0.9982\" C00=\"0.00168921\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.00168921\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"1375.79\" Tool=\"1\" X=\"-142.54\" Y=\"43.67\" Z=\"-1913.5\" QX=\"0.4478\" QY=\"-0.092\" QZ=\"-0.8824\" QR=\"0.1102\" C00=\"0.104782\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.104782\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"9948.11\" Tool=\"0\" X=\"-336.65\" Y=\"138.5\" Z=\"-2061.07\" QX=\"0.1251\" QY=\"-0.0638\" QZ=\"0.0071\" QR=\"0.99\" C00=\"0.023593\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.023593\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"9948.11\" Tool=\"1\" X=\"-202.09\" Y=\"120.33\" Z=\"-1949.81\" QX=\"0.4683\" QY=\"0.0188\" QZ=\"-0.8805\" QR=\"0.0696\" C00=\"0.0913248\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.0913248\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"104845\" Tool=\"0\" X=\"-134.86\" Y=\"295.49\" Z=\"-2187.63\" QX=\"0.1846\" QY=\"-0.2565\" QZ=\"-0.0829\" QR=\"0.945\" C00=\"0.022082\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.022082\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"104845\" Tool=\"1\" X=\"-56.93\" Y=\"233.79\" Z=\"-2042.6\" QX=\"-0.6264\" QY=\"-0.0197\" QZ=\"0.7772\" QR=\"0.0562\" C00=\"0.0915063\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.0915063\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
  "</Data>";

vnl_vector<mitk::ScalarType> tTool0Snapshot1(3);
vnl_vector<mitk::ScalarType> tTool1Snapshot2(3);
mitk::Quaternion qTool0Snapshot0;
mitk::Quaternion qTool1Snapshot1;

mitk::NavigationDataSequentialPlayer::Pointer player(
   mitk::NavigationDataSequentialPlayer::New());

void runLoop()
{
  mitk::NavigationData::Pointer nd0;
  mitk::NavigationData::Pointer nd1;
  for(unsigned int i=0; i<player->GetNumberOfSnapshots();++i)
  {
    player->Update();
    nd0 = player->GetOutput();
    nd1 = player->GetOutput(1);

    // test some values
    MITK_TEST_CONDITION_REQUIRED(nd0.IsNotNull(), "nd0.IsNotNull()");
    MITK_TEST_CONDITION_REQUIRED(nd1.IsNotNull(), "nd1.IsNotNull()");

    if(i==0)
    {
      MITK_TEST_CONDITION(qTool0Snapshot0.as_vector() == nd0->GetOrientation().as_vector(),
                          "qTool0Snapshot0.as_vector() == nd0->GetOrientation().as_vector()");
    }
    else if(i==1)
    {
      MITK_TEST_CONDITION(tTool0Snapshot1 == nd0->GetPosition().GetVnlVector(),
                          "tTool0Snapshot1 == nd0->GetPosition().GetVnlVector()");

      MITK_TEST_CONDITION(qTool1Snapshot1.as_vector() == nd1->GetOrientation().as_vector(),
                          "qTool1Snapshot1.as_vector() == nd1->GetOrientation().as_vector()");
    }
    else if(i==2) // should be repeated
    {
      MITK_TEST_CONDITION(tTool1Snapshot2 == nd1->GetPosition().GetVnlVector(),
                          "tTool1Snapshot2 == nd1->GetPosition().GetVnlVector()");
    }

  }
}

/**Documentation
 *  test for the class "NavigationDataRecorder".
 */
int mitkNavigationDataSequentialPlayerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataSequentialPlayer");

  // create test values valid for the xml data above
  tTool0Snapshot1[0] = -336.65;
  tTool0Snapshot1[1] = 138.5;
  tTool0Snapshot1[2]= -2061.07;
  tTool1Snapshot2[0] = -56.93;
  tTool1Snapshot2[1] = 233.79;
  tTool1Snapshot2[2]= -2042.6;
  vnl_vector_fixed<mitk::ScalarType,4> qVec;
  qVec[0] = 0.0085;
  qVec[1] = -0.0576;
  qVec[2]= -0.0022;
  qVec[3]= 0.9982;
  qTool0Snapshot0 = mitk::Quaternion(qVec);
  qVec[0] = 0.4683;
  qVec[1] = 0.0188;
  qVec[2]= -0.8805;
  qVec[3]= 0.0696;
  qTool1Snapshot1 = mitk::Quaternion(qVec);

  player
      ->SetXMLString(XML_STRING);

  MITK_TEST_CONDITION_REQUIRED(player->GetNumberOfSnapshots() == 3,
                               "player->GetNumberOfSnapshots() == 3");
  player
      ->SetRepeat(true);

  runLoop();
  // repeat is on should work a second time
  runLoop();

  // now test the go  to snapshot function
  player->GoToSnapshot(3);
  mitk::NavigationData::Pointer nd1 = player->GetOutput(1);
  MITK_TEST_CONDITION(tTool1Snapshot2 == nd1->GetPosition().GetVnlVector(),
                      "tTool1Snapshot2 == nd1->GetPosition().GetVnlVector()");

  player->GoToSnapshot(1);
  mitk::NavigationData::Pointer nd0 = player->GetOutput(0);
  MITK_TEST_CONDITION(qTool0Snapshot0.as_vector() == nd0->GetOrientation().as_vector(),
                      "qTool0Snapshot0.as_vector() == nd0->GetOrientation().as_vector()");

  player->GoToSnapshot(3);

  // and a third time
  runLoop();

  // always end with this!
  MITK_TEST_END();
}
