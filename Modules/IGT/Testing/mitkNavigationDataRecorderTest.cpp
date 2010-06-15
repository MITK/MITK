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

#include "mitkNavigationDataRecorder.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

#include <iostream>
#include <sstream>

/**Documentation
 *  test for the class "NavigationDataRecorder".
 */
int mitkNavigationDataRecorderTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataRecorder");
  std::string tmp = "";
  std::ostringstream* stream = new std::ostringstream( std::ostringstream::trunc );
  stream->setf( std::ios::fixed, std::ios::floatfield );

  // let's create an object of our class  
  mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(recorder.IsNotNull(), "Testing instantiation");

  MITK_TEST_CONDITION(recorder->GetInputs().size() == 0, "testing initial number of inputs"); 
  MITK_TEST_CONDITION(recorder->GetOutputs().size() == 0, "testing initial number of outputs");

  mitk::NavigationData::Pointer naviData = mitk::NavigationData::New();
  recorder->AddNavigationData( naviData );
  //recorder->SetFileName("e:/test");
  //recorder->SetRecordingMode( mitk::NavigationDataRecorder::NormalFile );
  recorder->StartRecording( stream );
  for ( unsigned int i=0; i<5; i++ )
  {
    mitk::Point3D pnt;
    pnt[0] = i + 1;
    pnt[1] = i + 1/2;
    pnt[2] = i +1*3;

    
    naviData->SetPosition(pnt);
    //naviData->Modified();
    recorder->Update();
    
    //Sleep(80 + i*10);

  }

  recorder->StopRecording();

  std::string str = stream->str();
  int pos = str.find( "ToolCount=" );
  std::string sub = stream->str().substr(pos+11, 1);
  MITK_TEST_CONDITION( sub.compare("1") == 0, "check if number of inputs is correct by stringstream");

  pos = str.find( "X=" );
  sub = stream->str().substr(pos+3, 1);
  MITK_TEST_CONDITION( sub.compare("1") == 0, "check if the X coordinate is correct");

  
  pos = str.find( "Y=" );
  sub = stream->str().substr(pos+3, 1);
  MITK_TEST_CONDITION( sub.compare("0") == 0, "check if the Y coordinate is correct");

  
  pos = str.find( "Z=" );
  sub = stream->str().substr(pos+3, 1);
  MITK_TEST_CONDITION( sub.compare("3") == 0, "check if the Z coordinate is correct");


  recorder->SetFileName("blablabla");
  const char* string = recorder->GetFileName();
  MITK_TEST_CONDITION( strcmp(string, "blablabla") == 0, "check if set- and getName-methods work");

  // always end with this!
  MITK_TEST_END();
}
