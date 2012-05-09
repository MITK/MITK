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

#include "mitkNavigationDataPlayer.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"
#include "mitkStandardFileLocations.h"
#include "mitkTimeStamp.h"

#include <iostream>
#include <sstream>

class mitkNavigationDataPlayerTestClass
  {
  public:
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

    std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestData.xml", "Modules/IGT/Testing/Data");

    player->SetFileName( file );

    MITK_TEST_CONDITION_REQUIRED( strcmp(player->GetFileName(), file.c_str()) == 0, "Testing SetFileName and GetFileName");

    player->SetStream( mitk::NavigationDataPlayer::NormalFile );
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
    player->SetFileName( file );
    player->SetStream( mitk::NavigationDataPlayer::NormalFile );

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

    mitk::TimeStamp::Pointer timer = mitk::TimeStamp::GetInstance();
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

    std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("NavigationDataTestData.xml", "Modules/IGT/Testing/Data");

    player->SetFileName( file );

    MITK_TEST_CONDITION_REQUIRED( strcmp(player->GetFileName(), file.c_str()) == 0, "Testing SetFileName and GetFileName");

    player->SetStream( mitk::NavigationDataPlayer::NormalFile );
    player->StartPlaying();
    player->Update();

    MITK_TEST_OUTPUT(<<"Test double call of Pause() method!");
    player->Pause(); //test pause method
    player->Pause(); //call again to see if this causes an error

    MITK_TEST_OUTPUT(<<"Test double call of Resume() method!");
    player->Resume(); //test resume method
    player->Resume(); //call again to see if this causes an error

    player->Update();
    player->StopPlaying();

    mitk::NavigationData::Pointer nd = player->GetOutput();
    mitk::Point3D pnt;
    pnt[0] = 1;
    pnt[1] = 0;
    pnt[2] = 3;

    MITK_TEST_CONDITION_REQUIRED( nd->GetPosition() == pnt, "Testing position of replayed NavigaionData" );

    player = mitk::NavigationDataPlayer::New();
    player->SetFileName( file );
    player->SetStream( mitk::NavigationDataPlayer::NormalFile );

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

    mitk::TimeStamp::Pointer timer = mitk::TimeStamp::GetInstance();
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
    player->SetStream( mitk::NavigationDataPlayer::ZipFile );
    player->StartPlaying();
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#0: Tested stream not set. Application should not crash.");

    //case 1: non-existing file
    player = mitk::NavigationDataPlayer::New();
    player->SetFileName( "ffdsd" );
    player->SetStream( mitk::NavigationDataPlayer::NormalFile );
    player->StartPlaying();
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#1: Tested non-existing file. Application should not crash.");

    //case 2: wrong file format
    player = mitk::NavigationDataPlayer::New();
    file = mitk::StandardFileLocations::GetInstance()->FindFile("SROMFile.rom", "Modules/IGT/Testing/Data");
    player->SetFileName( file );
    player->SetStream( mitk::NavigationDataPlayer::NormalFile );
    player->StartPlaying();
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#2: Tested wrong file format. Application should not crash.");

    //case 3: wrong file version
    player = mitk::NavigationDataPlayer::New();
    file = mitk::StandardFileLocations::GetInstance()->FindFile("InvalidVersionNavigationDataTestData.xml", "Modules/IGT/Testing/Data");
    player->SetFileName( file );
    player->SetStream( mitk::NavigationDataPlayer::NormalFile );
    player->StartPlaying();
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#3: Tested wrong file version. Application should not crash.");

    //case 4: wrong file
    player = mitk::NavigationDataPlayer::New();
    player->SetFileName( "cs:\fsd/$%§²³ffdsd" );
    player->SetStream( mitk::NavigationDataPlayer::NormalFile );
    player->StartPlaying();
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#4: Tested wrong file. Application should not crash.");

    //case 5: null stream
    player = mitk::NavigationDataPlayer::New();
    player->SetStream( NULL );
    player->StartPlaying();
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#5: Tested null stream. Application should not crash.");

    //case 6: empty stream
    player = mitk::NavigationDataPlayer::New();
    std::ifstream myEmptyStream = std::ifstream("");
    player->SetStream( &myEmptyStream );
    player->StartPlaying();
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#6: Tested empty stream. Application should not crash.");

    //case 7: wrong stream
    player = mitk::NavigationDataPlayer::New();
    file = mitk::StandardFileLocations::GetInstance()->FindFile("SROMFile.rom", "Modules/IGT/Testing/Data");
    std::ifstream myWrongStream = std::ifstream(file.c_str());
    player->SetStream( &myWrongStream );
    player->StartPlaying();
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#7: Tested wrong stream. Application should not crash.");

    //case 8: invalid
    player = mitk::NavigationDataPlayer::New();
    file = mitk::StandardFileLocations::GetInstance()->FindFile("InvalidDataNavigationDataTestData.xml", "Modules/IGT/Testing/Data");
    player->SetFileName( file );
    player->SetStream( mitk::NavigationDataPlayer::NormalFile );
    player->StartPlaying();
    player->Update();
    player->StopPlaying();
    MITK_TEST_OUTPUT(<<"#8: Tested invalid file version. Application should not crash.");



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
  mitkNavigationDataPlayerTestClass::TestPauseAndResume();
  mitkNavigationDataPlayerTestClass::TestInvalidStream();

  // always end with this!
  MITK_TEST_END();
}