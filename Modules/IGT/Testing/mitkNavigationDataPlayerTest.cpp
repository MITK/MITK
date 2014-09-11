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

#include "mitkNavigationDataPlayer.h"
#include "mitkNavigationData.h"
#include "mitkNavigationDataReaderXML.h"

#include "mitkTestingMacros.h"
#include "mitkStandardFileLocations.h"
#include "mitkIGTTimeStamp.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "mitkIGTException.h"
#include "mitkIGTIOException.h"



#include <itksys/SystemTools.hxx>

class mitkNavigationDataPlayerTestClass
  {
  public:
    static mitk::NavigationDataSet::Pointer GetNavigationDataSetFromXML(std::string filename)
    {
      mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
      return reader->Read(filename);
    }

    static void TestInstantiation()
    {
    // let's create an object of our class
    mitk::NavigationDataPlayer::Pointer player = mitk::NavigationDataPlayer::New();

    // first test: did this work?
    // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
    // it makes no sense to continue without an object.
    MITK_TEST_CONDITION_REQUIRED(player.IsNotNull(), "Testing instantiation");
    }

    static void TestSimpleDataPlay()
    {
    std::string tmp = "";

    // let's create an object of our class
    mitk::NavigationDataPlayer::Pointer player = mitk::NavigationDataPlayer::New();

    // create a file reader for the navigation data xml file
    mitk::NavigationDataReaderXML::Pointer navigationDataReader = mitk::NavigationDataReaderXML::New();
    std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestData.xml", "Modules/IGT/Testing/Data");

    // set NavigationDataSet to player
    mitk::NavigationDataSet::Pointer navigationDataSet = navigationDataReader->Read(file);
    player->SetNavigationDataSet( navigationDataSet );
    MITK_TEST_CONDITION_REQUIRED( navigationDataSet == player->GetNavigationDataSet() ,
                                  "Testing SetNavigationDataSet and GetNavigationDataSet." );

    player->StartPlaying();
    player->Update();
    player->StopPlaying();

    mitk::NavigationData::Pointer nd = player->GetOutput();
    mitk::Point3D pnt;
    pnt[0] = 1;
    pnt[1] = 0;
    pnt[2] = 3;

    MITK_TEST_CONDITION_REQUIRED( nd->GetPosition() == pnt, "Testing position of replayed NavigaionData" );

    player = mitk::NavigationDataPlayer::New();
    player->SetNavigationDataSet( navigationDataReader->Read(file) );

    std::vector<double> times, refTimes;
    refTimes.resize(5);
    refTimes[0] = 3.9;
    refTimes[1] = 83.6;
    refTimes[2] = 174.4;
    refTimes[3] = 275.0;
    refTimes[4] = 385.39;
    std::vector<mitk::Point3D> points, refPoints;
    refPoints.resize(5);
    refPoints[0][0] = 1; refPoints[0][1] = 0; refPoints[0][2] = 3;
    refPoints[1][0] = 2; refPoints[1][1] = 1; refPoints[1][2] = 4;
    refPoints[2][0] = 3; refPoints[2][1] = 2; refPoints[2][2] = 5;
    refPoints[3][0] = 4; refPoints[3][1] = 3; refPoints[3][2] = 6;
    refPoints[4][0] = 5; refPoints[4][1] = 4; refPoints[4][2] = 7;

    mitk::IGTTimeStamp::Pointer timer = mitk::IGTTimeStamp::GetInstance();
    timer->Initialize();

    itk::Object::Pointer obj = itk::Object::New();

    mitk::Point3D oldPos;
    oldPos[0] = 1;
    oldPos[1] = 0;
    oldPos[2] = 3;

    timer->Start( obj );
    player->StartPlaying();
    while( times.size()<5 )
    {
      player->Update();
      pnt = player->GetOutput()->GetPosition();
      if ( pnt != oldPos )
      {
        times.push_back( timer->GetElapsed(obj) );
        points.push_back(oldPos);
        oldPos = pnt;
      }
    }
    player->StopPlaying();

    // if this test fails, it may be because the dartclient runs on a virtual machine.
    // Under these circumstances, it may be impossible to achieve a time-accuracy of 10ms
    for ( int i=0;i<5;i++ )
      {
      if ((times[i]>refTimes[i]-150 && times[i]<refTimes[i]+150)) {MITK_TEST_OUTPUT(<< "ref: " << refTimes[i] << "  /  time elapsed: " << times[i]);}
      MITK_TEST_CONDITION_REQUIRED( (times[i]>refTimes[i]-150 && times[i]<refTimes[i]+150), "checking for more or less correct time-line"  );
      MITK_TEST_CONDITION_REQUIRED(points[i] == refPoints[i], "checking if the point coordinates are correct")
      }

    }

    static void TestPauseAndResume()
    {
    std::string tmp = "";

    // let's create an object of our class
    mitk::NavigationDataPlayer::Pointer player = mitk::NavigationDataPlayer::New();

    // create a file reader for the navigation data xml file
    mitk::NavigationDataReaderXML::Pointer navigationDataReader = mitk::NavigationDataReaderXML::New();
    std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestData.xml", "Modules/IGT/Testing/Data");

    // set NavigationDataSet to player
    player->SetNavigationDataSet( navigationDataReader->Read(file) );

    player->StartPlaying();
    player->Update();

    mitk::NavigationData::Pointer nd = player->GetOutput();
    mitk::Point3D pnt;
    pnt[0] = 1;
    pnt[1] = 0;
    pnt[2] = 3;

    MITK_TEST_CONDITION_REQUIRED( nd->GetPosition() == pnt, "Testing position of replayed NavigaionData" );

    MITK_TEST_OUTPUT(<<"Test double call of Pause() method!");
    player->Pause(); //test pause method
    player->Pause(); //call again to see if this causes an error

    MITK_TEST_OUTPUT(<<"Test double call of Resume() method!");
    player->Resume(); //test resume method
    player->Resume(); //call again to see if this causes an error

    player->Update();
    player->StopPlaying();

    player = mitk::NavigationDataPlayer::New();
    player->SetNavigationDataSet( navigationDataReader->Read(file) );

    std::vector<double> times, refTimes;
    refTimes.resize(5);
    refTimes[0] = 3.9;
    refTimes[1] = 83.6;
    refTimes[2] = 174.4;
    refTimes[3] = 275.0;
    refTimes[4] = 385.39;
    std::vector<mitk::Point3D> points, refPoints;
    refPoints.resize(5);
    refPoints[0][0] = 1; refPoints[0][1] = 0; refPoints[0][2] = 3;
    refPoints[1][0] = 2; refPoints[1][1] = 1; refPoints[1][2] = 4;
    refPoints[2][0] = 3; refPoints[2][1] = 2; refPoints[2][2] = 5;
    refPoints[3][0] = 4; refPoints[3][1] = 3; refPoints[3][2] = 6;
    refPoints[4][0] = 5; refPoints[4][1] = 4; refPoints[4][2] = 7;

    mitk::IGTTimeStamp::Pointer timer = mitk::IGTTimeStamp::GetInstance();
    timer->Initialize();

    itk::Object::Pointer obj = itk::Object::New();

    mitk::Point3D oldPos;
    oldPos[0] = 1;
    oldPos[1] = 0;
    oldPos[2] = 3;

    timer->Start( obj );
    player->StartPlaying();

    MITK_TEST_CONDITION_REQUIRED(!player->IsAtEnd(), "Testing method IsAtEnd() #0");

    while( times.size()<3 )
    {
      player->Update();
      pnt = player->GetOutput()->GetPosition();
      if ( pnt != oldPos )
      {
        times.push_back( timer->GetElapsed(obj) );
        points.push_back(oldPos);
        oldPos = pnt;
      }
    }
    MITK_TEST_OUTPUT(<<"Test pause method!");
    player->Pause();

    MITK_TEST_CONDITION_REQUIRED(!player->IsAtEnd(), "Testing method IsAtEnd() #1");

    MITK_TEST_OUTPUT(<<"Test resume method!");
    player->Resume();
    while( times.size()<5 )
    {
      player->Update();
      pnt = player->GetOutput()->GetPosition();
      if ( pnt != oldPos )
      {
        times.push_back( timer->GetElapsed(obj) );
        points.push_back(oldPos);
        oldPos = pnt;
      }
    }


    player->StopPlaying();

    // if this test fails, it may be because the dartclient runs on a virtual machine.
    // Under these circumstances, it may be impossible to achieve a time-accuracy of 10ms
    for ( int i=0;i<5;i++ )
      {
      if ((times[i]>refTimes[i]-150 && times[i]<refTimes[i]+150)) {MITK_TEST_OUTPUT(<< "ref: " << refTimes[i] << "  /  time elapsed: " << times[i]);}
      MITK_TEST_CONDITION_REQUIRED( (times[i]>refTimes[i]-150 && times[i]<refTimes[i]+150), "checking for more or less correct time-line"  );
      MITK_TEST_CONDITION_REQUIRED(points[i] == refPoints[i], "checking if the point coordinates are correct")
      }

    MITK_TEST_CONDITION_REQUIRED(player->IsAtEnd(), "Testing method IsAtEnd() #2");
    }

    static void TestInvalidStream()
    {
    MITK_TEST_OUTPUT(<<"#### Testing invalid input data: errors are expected. ####");

    //declarate test variables
    mitk::NavigationDataPlayer::Pointer player;
    std::string file;

    //case 0: stream not set
    player = mitk::NavigationDataPlayer::New();
    bool InvalidStreamException0 = false;
    try
    {
    player->StartPlaying();
    }
    catch(mitk::IGTException)
    {
    InvalidStreamException0=true;
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#0: Tested stream not set. Application should not crash.");
    }

    MITK_TEST_CONDITION_REQUIRED(InvalidStreamException0, "Testing Invalid Stream method if exception (stream not set) was thrown.");

    //case 1: non-existing file
    player = mitk::NavigationDataPlayer::New();
    bool InvalidStreamException1 = false;
    MITK_TEST_FOR_EXCEPTION(mitk::IGTIOException,
                            player->SetNavigationDataSet(mitkNavigationDataPlayerTestClass::GetNavigationDataSetFromXML(file)));
    try
    {
    player->StartPlaying();
    }
    catch(mitk::IGTException)
    {
    InvalidStreamException1=true;
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#1: Tested non-existing file. Application should not crash.");
    }

    MITK_TEST_CONDITION_REQUIRED(InvalidStreamException1, "Testing Invalid Stream method if exception (non-existing file) was thrown.");

    //case 2: wrong file format
    player = mitk::NavigationDataPlayer::New();
    bool InvalidStreamException2 = false;
    file = mitk::StandardFileLocations::GetInstance()->FindFile("SROMFile.rom", "Modules/IGT/Testing/Data");
    player->SetNavigationDataSet(mitkNavigationDataPlayerTestClass::GetNavigationDataSetFromXML(file));
    try
    {
    player->StartPlaying();
    }
    catch(mitk::IGTException)
    {
    InvalidStreamException2=true;
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#2: Tested wrong file format. Application should not crash.");
    }

    MITK_TEST_CONDITION_REQUIRED(InvalidStreamException2, "Testing Invalid Stream method if exception (wrong file format) was thrown.");

    //case 3: wrong file version
    player = mitk::NavigationDataPlayer::New();
    file = mitk::StandardFileLocations::GetInstance()->FindFile("InvalidVersionNavigationDataTestData.xml", "Modules/IGT/Testing/Data");
    player->SetNavigationDataSet(mitkNavigationDataPlayerTestClass::GetNavigationDataSetFromXML(file));
    bool InvalidStreamException3 = false;
    try
    {
    player->StartPlaying();
    }
    catch(mitk::IGTException)
    {
    InvalidStreamException3 = true;
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#3: Tested wrong file version. Application should not crash.");
    }
    MITK_TEST_CONDITION_REQUIRED(InvalidStreamException3, "Testing Invalid Stream method if exception (wrong file version) was thrown.");

    //case 4: wrong file
    mitk::NavigationDataSet::Pointer navigationDataSet;
    mitk::NavigationDataReaderXML::Pointer navigationDataReader = mitk::NavigationDataReaderXML::New();
    MITK_TEST_FOR_EXCEPTION(mitk::IGTIOException,
                            navigationDataSet = navigationDataReader->Read("cs:\fsd/$%§²³ffdsd"));

    player = mitk::NavigationDataPlayer::New();
    player->SetNavigationDataSet( navigationDataSet );

    bool InvalidStreamException4=false;
    try
    {
    player->StartPlaying();
    }
    catch(mitk::IGTException)
    {
    InvalidStreamException4=true;
    MITK_TEST_OUTPUT(<<"#4: Tested wrong file. Application should not crash.");
    }

    MITK_TEST_CONDITION_REQUIRED(InvalidStreamException4, "Testing Invalid Stream method if exception (wrong file) was thrown.");

    //case 5: null stream
    player = mitk::NavigationDataPlayer::New();
    bool InvalidStreamException5=false;
    try
    {
    player->StartPlaying();
    }
    catch(mitk::IGTException)
    {
    InvalidStreamException5=true;
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#5: Tested null stream. Application should not crash.");
    }

    MITK_TEST_CONDITION_REQUIRED(InvalidStreamException5, "Testing Invalid Stream method if exception (null stream) was thrown.");

    //case 6: empty stream, exception is thrown in setstream
    player = mitk::NavigationDataPlayer::New();
    bool InvalidStreamException6=false;
    std::ifstream* myEmptyStream;
    try
    {
      myEmptyStream = new std::ifstream("");
      mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
      reader->Read( myEmptyStream );
    }
    catch(mitk::IGTIOException)
    {
      InvalidStreamException6=true;
      MITK_TEST_OUTPUT(<<"#6: Tested empty stream. Application should not crash.");
    }
    MITK_TEST_CONDITION_REQUIRED(InvalidStreamException6, "Testing Invalid Stream method if exception (empty stream) was thrown.");




    //case 7: wrong stream
    player = mitk::NavigationDataPlayer::New();
    file = mitk::StandardFileLocations::GetInstance()->FindFile("SROMFile.rom", "Modules/IGT/Testing/Data");

    bool InvalidStreamException7=false;
    std::ifstream* myWrongStream;
    myWrongStream = new std::ifstream(file.c_str());
    try
    {
      mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
      reader->Read( myWrongStream );
    }
    catch(mitk::IGTIOException)
    {
    InvalidStreamException7=true;
    MITK_TEST_OUTPUT(<<"#7: Tested wrong stream. Application should not crash.");
    }

    MITK_TEST_CONDITION_REQUIRED(InvalidStreamException7, "Testing Invalid Stream method if exception (wrong stream) was thrown.");



    //case 8: invalid
    player = mitk::NavigationDataPlayer::New();
    file = mitk::StandardFileLocations::GetInstance()->FindFile("InvalidDataNavigationDataTestData.xml", "Modules/IGT/Testing/Data");
    bool InvalidStreamException8=false;
    try
    {
      mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
      reader->Read( myWrongStream );
      player->SetNavigationDataSet( reader->Read( file ) );
      player->StartPlaying();
    }
    catch(mitk::IGTIOException)
    {
    InvalidStreamException8=true;
    MITK_TEST_OUTPUT(<<"#8: Tested invalid file version. Application should not crash.");
    }
    MITK_TEST_CONDITION_REQUIRED(InvalidStreamException8, "Testing Invalid Stream method if exception (Invalid) was thrown.");

    //clean up
    delete myEmptyStream;
    delete myWrongStream;
    }

  static void TestSetStreamExceptions()
    {
    mitk::NavigationDataPlayer::Pointer myTestPlayer = mitk::NavigationDataPlayer::New();
    std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestData.xml", "Modules/IGT/Testing/Data");

    myTestPlayer->SetNavigationDataSet( mitkNavigationDataPlayerTestClass::GetNavigationDataSetFromXML( file ) );

    bool exceptionThrown=false;

    try
    {
      std::istream* stream = NULL;
      mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
      reader->Read( stream );
    }
    catch(mitk::IGTIOException)
    {
     exceptionThrown = true;
     MITK_TEST_OUTPUT(<<"#9: Tested exceptions in SetStream. Application should not crash.");

    }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown, "Testing SetStream method in exception was thrown.");

  }

 static void TestStartPlayingExceptions()
    {
      MITK_INFO <<"In the following, exceptions are tested. Errors will occur and are expected.";


      //Case1 Testing if stream=NULL
      mitk::NavigationDataPlayer::Pointer myTestPlayer1 = mitk::NavigationDataPlayer::New();
      bool exceptionThrown1 = false;
      try
       {
       myTestPlayer1->StartPlaying();

       }
       catch(mitk::IGTException)
       {
       exceptionThrown1 = true;
       myTestPlayer1->StopPlaying();
       MITK_TEST_OUTPUT(<<"#10: Tested exception for the case when stream=NULL in StartPlaying. Application should not crash.");
       }

      MITK_TEST_CONDITION_REQUIRED(exceptionThrown1, "Testing StartPlaying method if exception (stream=NULL) was thrown.");

      //Case2 Testing if file does not exist
      mitk::NavigationDataPlayer::Pointer myTestPlayer2 = mitk::NavigationDataPlayer::New();

      MITK_TEST_FOR_EXCEPTION(mitk::IGTIOException,
                              myTestPlayer2->SetNavigationDataSet( mitkNavigationDataPlayerTestClass::GetNavigationDataSetFromXML("ffdsd") ));
      bool exceptionThrown2 = false;
      try{
      myTestPlayer2->StartPlaying();
      }
      catch(mitk::IGTException)
      {
      exceptionThrown2 = true;
      myTestPlayer2->StopPlaying();
      MITK_TEST_OUTPUT(<<"#11: Tested exception for the case when file does not exist in StartPlaying. Application should not crash.");
      }
      MITK_TEST_CONDITION_REQUIRED(exceptionThrown2, "Testing StartPlaying method if exception is thrown when file does not exist.");


      //Case3 Testing if wrong file format
      mitk::NavigationDataPlayer::Pointer myTestPlayer3 = mitk::NavigationDataPlayer::New();
      std::string file3 = mitk::StandardFileLocations::GetInstance()->FindFile("SROMFile.rom", "Modules/IGT/Testing/Data");

      bool exceptionThrown3 = false;
      try
      {
        myTestPlayer3->SetNavigationDataSet( mitkNavigationDataPlayerTestClass::GetNavigationDataSetFromXML(file3) );
      }
      catch(mitk::IGTIOException)
      {
        MITK_TEST_OUTPUT(<<"#12: Tested exception for the case when file format is wrong. Application should not crash.");
        exceptionThrown3 = true;
      }
      MITK_TEST_CONDITION_REQUIRED(exceptionThrown3, "Testing StartPlaying method if exception (file format is wrong) was thrown.");

      //Case4 Testing if wrong file version
      mitk::NavigationDataPlayer::Pointer myTestPlayer4 = mitk::NavigationDataPlayer::New();
      std::string file4 = mitk::StandardFileLocations::GetInstance()->FindFile("InvalidVersionNavigationDataTestData.xml", "Modules/IGT/Testing/Data");
      bool exceptionThrown4 = false;
      try
      {
        mitk::NavigationDataSet::Pointer navigationDataSet
            = mitkNavigationDataPlayerTestClass::GetNavigationDataSetFromXML(file4);
        myTestPlayer4->SetNavigationDataSet( navigationDataSet );
      }
      catch(mitk::IGTIOException)
      {
        exceptionThrown4 = true;
        MITK_TEST_OUTPUT(<<"#13: Tested exception for the case when file version is wrong in StartPlaying. Application should not crash.");
      }
      MITK_TEST_CONDITION_REQUIRED(exceptionThrown4, "Testing StartPlaying method if exception (file version is wrong) was thrown.");

      //Case5 Testing if not existing file name
      mitk::NavigationDataPlayer::Pointer myTestPlayer5 = mitk::NavigationDataPlayer::New();
      bool exceptionThrown5 = false;

      try
      {
        mitk::NavigationDataSet::Pointer navigationDataSet
            = mitkNavigationDataPlayerTestClass::GetNavigationDataSetFromXML("ffdsd");
        myTestPlayer4->SetNavigationDataSet( navigationDataSet );
      }
      catch(mitk::IGTIOException)
      {
        exceptionThrown5 = true;
        MITK_TEST_OUTPUT(<<"#14: Tested exception for the case when non-existing file name in StartPlaying. Application should not crash.");
      }
      MITK_TEST_CONDITION_REQUIRED(exceptionThrown5, "Testing StartPlaying method if exception (non-existing file name) was thrown.");

 }

};

/**Documentation
 *  test for the class "NavigationDataPlayer".
 */

int mitkNavigationDataPlayerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataPlayer");
  std::string tmp = "";

  mitkNavigationDataPlayerTestClass::TestInstantiation();
  mitkNavigationDataPlayerTestClass::TestSimpleDataPlay();
  mitkNavigationDataPlayerTestClass::TestSetStreamExceptions();
  //mitkNavigationDataPlayerTestClass::TestStartPlayingExceptions();
  mitkNavigationDataPlayerTestClass::TestPauseAndResume();
  //mitkNavigationDataPlayerTestClass::TestInvalidStream();

  // always end with this!
  MITK_TEST_END();
}



