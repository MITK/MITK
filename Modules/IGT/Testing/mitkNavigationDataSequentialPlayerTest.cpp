
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

//foe exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"


const char* XML_STRING =
  "<?xml version=\"1.0\" ?><Version Ver=\"1\" /><Data ToolCount=\"2\">"
    "<NavigationData Time=\"1375.79\" Tool=\"0\" X=\"-279.14\" Y=\"40.48\" Z=\"-2023.72\" QX=\"0.0085\" QY=\"-0.0576\" QZ=\"-0.0022\" QR=\"0.9982\" C00=\"0.00168921\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.00168921\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"1375.79\" Tool=\"1\" X=\"-142.54\" Y=\"43.67\" Z=\"-1913.5\" QX=\"0.4478\" QY=\"-0.092\" QZ=\"-0.8824\" QR=\"0.1102\" C00=\"0.104782\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.104782\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"9948.11\" Tool=\"0\" X=\"-336.65\" Y=\"138.5\" Z=\"-2061.07\" QX=\"0.1251\" QY=\"-0.0638\" QZ=\"0.0071\" QR=\"0.99\" C00=\"0.023593\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.023593\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"9948.11\" Tool=\"1\" X=\"-202.09\" Y=\"120.33\" Z=\"-1949.81\" QX=\"0.4683\" QY=\"0.0188\" QZ=\"-0.8805\" QR=\"0.0696\" C00=\"0.0913248\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.0913248\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"104845\" Tool=\"0\" X=\"-134.86\" Y=\"295.49\" Z=\"-2187.63\" QX=\"0.1846\" QY=\"-0.2565\" QZ=\"-0.0829\" QR=\"0.945\" C00=\"0.022082\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.022082\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationData Time=\"104845\" Tool=\"1\" X=\"-56.93\" Y=\"233.79\" Z=\"-2042.6\" QX=\"-0.6264\" QY=\"-0.0197\" QZ=\"0.7772\" QR=\"0.0562\" C00=\"0.0915063\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.0915063\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
  "</Data>";

const char* XML_INVALID_TESTSTRING =
  "<?version=\"1.0\" ?><Version Ver=\"1\" />< ToolCount=\"2\">"
    "<NavigationDhgata Time=\"1375.79\" Tool=\"0\" X=\"-279.14\" Y=\"40.48\" Z=\"-2023.72\" QX=\"0.0085\" QY=\"-0.0576\" QZ=\"-0.0022\" QR=\"0.9982\" C00=\"0.00168921\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.00168921\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigatifhgonData Time=\"1375.79\" Tool=\"1\" X=\"-142.54\" Y=\"43.67\" Z=\"-1913.5\" QX=\"0.4478\" QY=\"-0.092\" QZ=\"-0.8824\" QR=\"0.1102\" C00=\"0.104782\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.104782\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigathgfionData Time=\"9948.11\" Tool=\"0\" X=\"-336.65\" Y=\"138.5\" Z=\"-2061.07\" QX=\"0.1251\" QY=\"-0.0638\" QZ=\"0.0071\" QR=\"0.99\" C00=\"0.023593\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.023593\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigatifghonData Time=\"9948.11\" Tool=\"1\" X=\"-202.09\" Y=\"120.33\" Z=\"-1949.81\" QX=\"0.4683\" QY=\"0.0188\" QZ=\"-0.8805\" QR=\"0.0696\" C00=\"0.0913248\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.0913248\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigatgfhionData Time=\"104845\" Tool=\"0\" X=\"-134.86\" Y=\"295.49\" Z=\"-2187.63\" QX=\"0.1846\" QY=\"-0.2565\" QZ=\"-0.0829\" QR=\"0.945\" C00=\"0.022082\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.022082\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
    "<NavigationDatja Time=\"104845\" Tool=\"1\" X=\"-56.93\" Y=\"233.79\" Z=\"-2042.6\" QX=\"-0.6264\" QY=\"-0.0197\" QZ=\"0.7772\" QR=\"0.0562\" C00=\"0.0915063\" C01=\"0\" C02=\"0\" C03=\"0\" C04=\"0\" C05=\"0\" C10=\"0\" C11=\"0.0915063\" C12=\"0\" C13=\"0\" C14=\"0\" C15=\"0\" Valid=\"1\" hO=\"1\" hP=\"1\" />"
  "</>";

vnl_vector<mitk::ScalarType> tTool0Snapshot1(3);
vnl_vector<mitk::ScalarType> tTool1Snapshot2(3);
mitk::Quaternion qTool0Snapshot0;
mitk::Quaternion qTool1Snapshot1;

mitk::NavigationDataSequentialPlayer::Pointer player(
   mitk::NavigationDataSequentialPlayer::New());

bool runLoop()
{


  bool success = true;
  mitk::NavigationData::Pointer nd0;
  mitk::NavigationData::Pointer nd1;
  for(unsigned int i=0; i<player->GetNumberOfSnapshots();++i)
  {
    player->Update();
    nd0 = player->GetOutput();
    nd1 = player->GetOutput(1);

    // test some values
    if(nd0.IsNull() || nd1.IsNull()) return false;

    if(i==0)
    {
      if (!(qTool0Snapshot0.as_vector() == nd0->GetOrientation().as_vector())) {success = false;}
    }
    else if(i==1)
    {
      if (!(tTool0Snapshot1 == nd0->GetPosition().GetVnlVector())) {success = false;}
      else if (!(qTool1Snapshot1.as_vector() == nd1->GetOrientation().as_vector())) {success = false;}
    }
    else if(i==2) // should be repeated
    {
      if (!(tTool1Snapshot2 == nd1->GetPosition().GetVnlVector())) {success = false;}
    }

  }
  return success;
}

void TestStandardWorkflow()
{
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

  //test SetXMLString()
  player->SetXMLString(XML_STRING);
  MITK_TEST_CONDITION_REQUIRED(player->GetNumberOfSnapshots() == 3,"Testing method SetXMLString with 3 navigation datas.");

  //rest repeat
  player->SetRepeat(true);
  MITK_TEST_CONDITION_REQUIRED(runLoop(),"Testing first run.");
  MITK_TEST_CONDITION_REQUIRED(runLoop(),"Testing second run."); //repeat is on should work a second time

  // now test the go to snapshot function
  player->GoToSnapshot(3);
  mitk::NavigationData::Pointer nd1 = player->GetOutput(1);
  MITK_TEST_CONDITION(tTool1Snapshot2 == nd1->GetPosition().GetVnlVector(),
                      "Testing GoToSnapshot() [1]");

  player->GoToSnapshot(1);
  mitk::NavigationData::Pointer nd0 = player->GetOutput(0);
  MITK_TEST_CONDITION(qTool0Snapshot0.as_vector() == nd0->GetOrientation().as_vector(),
                      "Testing GoToSnapshot() [2]");

  player->GoToSnapshot(3);

  // and a third time
  MITK_TEST_CONDITION_REQUIRED(runLoop(),"Tested if repeat works again.");

}

void TestSetFileNameException()
{ //testing exception if file name hasnt been set
  mitk::NavigationDataSequentialPlayer::Pointer myTestPlayer = mitk::NavigationDataSequentialPlayer::New();
  bool exceptionThrown=false;
  try
  {
    myTestPlayer->SetFileName("");
  }
  catch(mitk::IGTIOException)
  {
   exceptionThrown=true;
    MITK_TEST_OUTPUT(<<"Tested exception for the case when file version is wrong in SetFileName. Application should not crash.");
  }
  MITK_TEST_CONDITION_REQUIRED(exceptionThrown, "Testing SetFileName method if exception (if file name hasnt been set) was thrown.");

  //testing ReInItXML method if data element is not found
  mitk::NavigationDataSequentialPlayer::Pointer myTestPlayer1 = mitk::NavigationDataSequentialPlayer::New();
  std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestDataInvalidTags.xml", "Modules/IGT/Testing/Data");
  bool exceptionThrown1=false;
  try
  {
    myTestPlayer1->SetFileName(file);
  }
  catch(mitk::IGTException)
  {
   exceptionThrown1=true;
  }
  MITK_TEST_CONDITION_REQUIRED(exceptionThrown1, "Testing SetFileName method if exception (if data element not found) was thrown.");

}

void TestGoToSnapshotException()
{
 //testing GoToSnapShot for exception
  mitk::NavigationDataSequentialPlayer::Pointer myTestPlayer2 = mitk::NavigationDataSequentialPlayer::New();
  myTestPlayer2->SetXMLString(XML_STRING);

  bool exceptionThrown2=false;
  try
  {
    myTestPlayer2->GoToSnapshot(1000);
  }
  catch(mitk::IGTException)
  {
    exceptionThrown2=true;
  }
  MITK_TEST_CONDITION_REQUIRED(exceptionThrown2, "Testing if exception is thrown when GoToSnapShot method is called with an index that doesn't exist.");
}

void TestSetXMLStringException()
{
  mitk::NavigationDataSequentialPlayer::Pointer myTestPlayer3 = mitk::NavigationDataSequentialPlayer::New();

  bool exceptionThrown3=false;

  //The string above XML_INVALID_TESTSTRING is a wrong string, some element were deleted in above
  try
  {
    myTestPlayer3->SetXMLString(XML_INVALID_TESTSTRING);
  }
  catch(mitk::IGTException)
  {
    exceptionThrown3=true;
  }
 MITK_TEST_CONDITION_REQUIRED(exceptionThrown3, "Testing SetXMLString method with an invalid XML string.");
}




/**Documentation
 *  test for the class "NavigationDataRecorder".
 */
int mitkNavigationDataSequentialPlayerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataSequentialPlayer");

  TestStandardWorkflow();
  TestSetFileNameException();
  TestSetXMLStringException();
  TestGoToSnapshotException();

  MITK_TEST_END();
}
