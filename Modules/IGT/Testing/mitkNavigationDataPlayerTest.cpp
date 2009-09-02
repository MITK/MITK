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
  std::ostringstream* stream = new std::ostringstream( std::ostringstream::trunc );

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
  //MITK_TEST_CONDITION_REQUIRED( nd->GetTimeStamp() == 3068.94, "Testing for correct TimeStamp" );


  std::vector<double> times, refTimes;
  refTimes.resize(5);
  refTimes[0] = 3.9;
  refTimes[1] = 83.6;
  refTimes[2] = 174.4;
  refTimes[3] = 275.0;
  refTimes[4] = 385.39;

  mitk::TimeStamp::Pointer timer = mitk::TimeStamp::GetInstance();
  timer->Initialize();

  itk::Object::Pointer obj = itk::Object::New();
  timer->Start( obj );

  mitk::Point3D oldPos;
  oldPos[0] = 1;
  oldPos[1] = 0;
  oldPos[2] = 3;
  //pnt = oldPos;
  player->StartPlaying();
  while( times.size()<5 )
  {
    player->Update();
    pnt = nd->GetPosition();
    if ( pnt != oldPos )
    {
      times.push_back( timer->GetElapsed(obj) );
      oldPos = pnt;
    }
  }

  // if this test fails, it may be because the dartclient runs on a virtual machine.
  // Under these circumstances, it may be impossible to achieve a time-accuracy of 10ms
  for ( int i=0;i<5;i++ )
  {
    std::cout << "ref: " << refTimes[i] << "  /  time elapsed: " << times[i] << std::endl;
    MITK_TEST_CONDITION_REQUIRED( (times[i]>refTimes[i]-10 && times[i]<refTimes[i]+10), "checking for more or less correct time-line"  );
  }


  // always end with this!
  MITK_TEST_END();
}
