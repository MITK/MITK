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
#include <mitkTestFixture.h>
#include "mitkNavigationDataReaderXML.h"

#include <iostream>
#include <sstream>

//foe exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

class mitkNavigationDataSequentialPlayerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationDataSequentialPlayerTestSuite);
  MITK_TEST(TestStandardWorkflow);
  MITK_TEST(TestRestartWithNewNavigationDataSet);
  MITK_TEST(TestSetFileNameException);
  MITK_TEST(TestGoToSnapshotException);
  MITK_TEST(TestSetXMLStringException);
  MITK_TEST(TestDoubleUpdate);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  vnl_vector<mitk::ScalarType> tTool0Snapshot1;
  vnl_vector<mitk::ScalarType> tTool1Snapshot2;
  mitk::Quaternion qTool0Snapshot0;
  mitk::Quaternion qTool1Snapshot1;

  mitk::NavigationDataSequentialPlayer::Pointer player;

public:

  void setUp(){
    tTool0Snapshot1 = vnl_vector<mitk::ScalarType>(3);
    tTool1Snapshot2 = vnl_vector<mitk::ScalarType>(3);
    mitk::Quaternion qTool0Snapshot0;
    mitk::Quaternion qTool1Snapshot1;

    player = mitk::NavigationDataSequentialPlayer::New();
  }

  void tearDown()
  {
  }

  bool runLoop()
  {
    bool success = true;
    mitk::NavigationData::Pointer nd0;
    mitk::NavigationData::Pointer nd1;
    for(unsigned int i=0; i<player->GetNumberOfSnapshots(); ++i)
    {
      player->GoToNextSnapshot();
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
    std::string file = GetTestDataFilePath("IGT-Data/NavigationDataTestData_2ToolsDouble.xml");
    mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
    player->SetNavigationDataSet(reader->Read(file));

    MITK_TEST_CONDITION(player->GetNumberOfSnapshots() == 3,"Testing method SetXMLString with 3 navigation datas.");
    MITK_TEST_CONDITION(player->GetNumberOfIndexedOutputs() == 2,"Testing number of outputs");

    //rest repeat
    player->SetRepeat(true);
    MITK_TEST_CONDITION(runLoop(),"Testing first run.");
    MITK_TEST_CONDITION(runLoop(),"Testing second run."); //repeat is on should work a second time

    // now test the go to snapshot function
    player->GoToSnapshot(2);
    mitk::NavigationData::Pointer nd1 = player->GetOutput(1);
    MITK_TEST_CONDITION(tTool1Snapshot2 == nd1->GetPosition().GetVnlVector(),
      "Testing GoToSnapshot() [1]");

    MITK_TEST_OUTPUT( << tTool1Snapshot2 << "\t" << nd1->GetPosition().GetVnlVector());

    player->GoToSnapshot(0);
    mitk::NavigationData::Pointer nd0 = player->GetOutput();
    MITK_TEST_CONDITION(qTool0Snapshot0.as_vector() == nd0->GetOrientation().as_vector(),
      "Testing GoToSnapshot() [2]");

    MITK_TEST_OUTPUT( << qTool0Snapshot0.as_vector() << "\t" <<nd0->GetOrientation().as_vector() );

    player->GoToSnapshot(2);

    // and a third time
    MITK_TEST_CONDITION(runLoop(),"Tested if repeat works again.");
  }

  void TestRestartWithNewNavigationDataSet()
  {
    mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();

    mitk::NavigationDataSequentialPlayer::Pointer player(mitk::NavigationDataSequentialPlayer::New());

    std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestData_2ToolsDouble.xml", "Modules/IGT/Testing/Data");

    player->SetNavigationDataSet(reader->Read(file));
    mitk::NavigationData::PositionType nd1 = player->GetOutput(0)->GetPosition();
    player->SetNavigationDataSet(reader->Read(file));
    player->Update();
    mitk::NavigationData::PositionType nd2 = player->GetOutput(0)->GetPosition();

    MITK_TEST_CONDITION(nd1 == nd2, "First output must be the same after setting same navigation data again.");

    // setting new NavigationDataSet with different tool count should result in an exception
    file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestData.xml", "Modules/IGT/Testing/Data");
    MITK_TEST_FOR_EXCEPTION(mitk::IGTException, player->SetNavigationDataSet(reader->Read(file)));
  }

  void TestSetFileNameException()
  { //testing exception if file name hasnt been set
    mitk::NavigationDataSequentialPlayer::Pointer myTestPlayer = mitk::NavigationDataSequentialPlayer::New();
    bool exceptionThrown=false;
    try
    {
      mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
      myTestPlayer->SetNavigationDataSet(reader->Read(""));
    }
    catch(mitk::IGTIOException)
    {
      exceptionThrown=true;
      MITK_TEST_OUTPUT(<<"Tested exception for the case when file version is wrong in SetFileName. Application should not crash.");
    }
    MITK_TEST_CONDITION(exceptionThrown, "Testing SetFileName method if exception (if file name hasnt been set) was thrown.");

    //testing ReInItXML method if data element is not found
    mitk::NavigationDataSequentialPlayer::Pointer myTestPlayer1 = mitk::NavigationDataSequentialPlayer::New();
    std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestDataInvalidTags.xml", "Modules/IGT/Testing/Data");
    bool exceptionThrown1=false;
    try
    {
      mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
      myTestPlayer1->SetNavigationDataSet(reader->Read(file));
    }
    catch(mitk::IGTException)
    {
      exceptionThrown1=true;
    }
    MITK_TEST_CONDITION(exceptionThrown1, "Testing SetFileName method if exception (if data element not found) was thrown.");
  }

  void TestGoToSnapshotException()
  {
    //testing GoToSnapShot for exception
    mitk::NavigationDataSequentialPlayer::Pointer myTestPlayer2 = mitk::NavigationDataSequentialPlayer::New();
    std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestData_2Tools.xml", "Modules/IGT/Testing/Data");
    mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
    myTestPlayer2->SetNavigationDataSet(reader->Read(file));

    bool exceptionThrown2=false;
    try
    {
      unsigned int invalidSnapshot = 1000;
      myTestPlayer2->GoToSnapshot(invalidSnapshot);
    }
    catch(mitk::IGTException)
    {
      exceptionThrown2=true;
    }
    MITK_TEST_CONDITION(exceptionThrown2, "Testing if exception is thrown when GoToSnapShot method is called with an index that doesn't exist.");
  }

  void TestSetXMLStringException()
  {
    mitk::NavigationDataSequentialPlayer::Pointer myTestPlayer3 = mitk::NavigationDataSequentialPlayer::New();

    bool exceptionThrown3=false;

    //The string above XML_INVALID_TESTSTRING is a wrong string, some element were deleted in above
    try
    {
      std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("InvalidVersionNavigationDataTestData.xml", "Modules/IGT/Testing/Data");
      mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
      myTestPlayer3->SetNavigationDataSet(reader->Read(file));
    }
    catch(mitk::IGTIOException)
    {
      exceptionThrown3=true;
    }
    MITK_TEST_CONDITION(exceptionThrown3, "Testing SetXMLString method with an invalid XML string.");
  }

  void TestDoubleUpdate()
  {
    std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestData_2Tools.xml", "Modules/IGT/Testing/Data");

    mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
    player->SetNavigationDataSet(reader->Read(file));

    player->Update();
    mitk::Quaternion nd1Orientation = player->GetOutput()->GetOrientation();

    player->Update();
    mitk::Quaternion nd2Orientation = player->GetOutput()->GetOrientation();

    MITK_TEST_CONDITION(nd1Orientation.as_vector() == nd2Orientation.as_vector(), "Output must be the same no matter if Update() was called between.");

    MITK_TEST_CONDITION(player->GoToNextSnapshot(), "There must be a next snapshot available.");
    player->Update();
    mitk::Quaternion nd3Orientation = player->GetOutput()->GetOrientation();

    MITK_TEST_CONDITION(nd1Orientation.as_vector() != nd3Orientation.as_vector(), "Output must be different if GoToNextSnapshot() was called between.");
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationDataSequentialPlayer)