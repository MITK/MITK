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
  MITK_TEST(TestGoToSnapshotException);
  MITK_TEST(TestDoubleUpdate);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::NavigationDataSet::Pointer NavigationDataSet;
  mitk::NavigationDataSequentialPlayer::Pointer player;

public:

  void setUp(){
    player = mitk::NavigationDataSequentialPlayer::New();
    std::string file = GetTestDataFilePath("IGT-Data/NavigationDataTestData_2ToolsDouble.xml");
    mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
    NavigationDataSet =reader->Read(file);
  }

  void tearDown()
  {
  }

  bool runLoop()
  {
    player->Update();
    mitk::NavigationData::Pointer nd0;
    mitk::NavigationData::Pointer nd1;
    for(unsigned int i=0; i<player->GetNumberOfSnapshots(); ++i)
    {
      nd0 = player->GetOutput(0);
      nd1 = player->GetOutput(1);

      // test some values
      if(nd0.IsNull() || nd1.IsNull()) return false;

      //Compare data
      mitk::NavigationData::Pointer ref0 = NavigationDataSet->GetNavigationDataForIndex(i,0);
      mitk::NavigationData::Pointer ref1 = NavigationDataSet->GetNavigationDataForIndex(i,1);
      if (!(ref0->GetOrientation().as_vector() == nd0->GetOrientation().as_vector())) {return false;}
      if (!(ref1->GetOrientation().as_vector() == nd1->GetOrientation().as_vector())) {return false;}
      if (!(ref0->GetPosition().GetVnlVector() == nd0->GetPosition().GetVnlVector())) {return false;}
      if (!(ref1->GetPosition().GetVnlVector() == nd1->GetPosition().GetVnlVector())) {return false;}

      // Goto next Snapshot
      player->GoToNextSnapshot();
    }
    return true;
  }

  void TestStandardWorkflow()
  {
    // Set NavigationDatas for player
    player->SetNavigationDataSet(NavigationDataSet);

    MITK_TEST_CONDITION(player->GetNumberOfSnapshots() == 3,"Testing if player reports correct number of Snapshots");
    MITK_TEST_CONDITION(player->GetNumberOfIndexedOutputs() == 2,"Testing number of outputs");

    //rest repeat
    player->SetRepeat(true);

    MITK_TEST_CONDITION(runLoop(),"Testing first run.");
    MITK_TEST_CONDITION(runLoop(),"Testing second run."); //repeat is on should work a second time

    // now test the go to snapshot function
    player->GoToSnapshot(2);
    mitk::NavigationData::Pointer nd1 = player->GetOutput(1);
    mitk::NavigationData::Pointer ref1 = NavigationDataSet->GetNavigationDataForIndex(2,1);
    MITK_TEST_CONDITION(ref1->GetPosition().GetVnlVector() == nd1->GetPosition().GetVnlVector(),
      "Testing GoToSnapshot() [1]");

    //MITK_TEST_OUTPUT( << "Reference:" << ref1->GetPosition().GetVnlVector() << "\tObserved: " << nd1->GetPosition().GetVnlVector());

    player->GoToSnapshot(0);
    mitk::NavigationData::Pointer nd0 = player->GetOutput();
    mitk::NavigationData::Pointer ref0 = NavigationDataSet->GetNavigationDataForIndex(0,0);
    MITK_TEST_CONDITION(ref0->GetOrientation().as_vector() == nd0->GetOrientation().as_vector(),
      "Testing GoToSnapshot() [2]");

    //MITK_TEST_OUTPUT( << "Reference" << ref0->GetPosition().GetVnlVector() << "\tObserved:" <<nd0->GetOrientation().as_vector() );
  }

  void TestRestartWithNewNavigationDataSet()
  {
    player->SetNavigationDataSet(NavigationDataSet);
    mitk::NavigationData::PositionType nd1 = player->GetOutput(0)->GetPosition();
    player->SetNavigationDataSet(NavigationDataSet);
    mitk::NavigationData::PositionType nd2 = player->GetOutput(0)->GetPosition();

    MITK_TEST_CONDITION(nd1 == nd2, "First output must be the same after setting same navigation data again.");

    // setting new NavigationDataSet with different tool count should result in an exception
    std::string file = GetTestDataFilePath("IGT-Data/NavigationDataTestData.xml");
    mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
    MITK_TEST_FOR_EXCEPTION(mitk::IGTException, player->SetNavigationDataSet(reader->Read(file)));
  }

  void TestGoToSnapshotException()
  {
    //testing GoToSnapShot for exception
    mitk::NavigationDataSequentialPlayer::Pointer myTestPlayer2 = mitk::NavigationDataSequentialPlayer::New();
    std::string file = GetTestDataFilePath("IGT-Data/NavigationDataTestData_2Tools.xml");
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

  void TestDoubleUpdate()
  {
    //std::string file = GetTestDataFilePath("IGT-Data/NavigationDataTestData_2Tools.xml");

    //mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
    //player->SetNavigationDataSet(reader->Read(file));
    player->SetNavigationDataSet(NavigationDataSet);

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