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

/**Documentation
 *  test for the class "NavigationDataPlayer".
 */
int mitkNavigationDataPlayerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataPlayer");
  std::string tmp = "";

  // let's create an object of our class  
  mitk::NavigationDataPlayer::Pointer player = mitk::NavigationDataPlayer::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(player.IsNotNull(), "Testing instantiation");

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

  // always end with this!
  MITK_TEST_END();
}
