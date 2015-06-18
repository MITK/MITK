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

#include "mitkIGTTimeStamp.h"
#include "mitkRealTimeClock.h"

#include "mitkTestingMacros.h"
#include "itkObject.h"

//#include <time.h>

#ifndef WIN32
#include <unistd.h>
#endif



/**
 *  test for the class "mitkIGTTimeStamp"
 */
int mitkTimeStampTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("IGTTimeStamp")

  //create testing objects
  itk::Object::Pointer tester = itk::Object::New();
  itk::Object::Pointer tester2 = itk::Object::New();

  //try methods without calling CreateInstance() before (error test cases)
  MITK_TEST_OUTPUT(<<"Testing behavior of methodes if they are called at the wrong moment (error should occure).");
  mitk::IGTTimeStamp::GetInstance()->Start(tester2);
  mitk::IGTTimeStamp::GetInstance()->Stop(tester2);

  //create instance of singleton time stamp class
  MITK_TEST_CONDITION_REQUIRED(mitk::IGTTimeStamp::CreateInstance() != (nullptr), "Testing GetInstance() of Singleton" );

  double timestampX = mitk::IGTTimeStamp::GetInstance()->GetElapsed();
  MITK_TEST_CONDITION_REQUIRED(timestampX==-1,"..Testing method GetCurrentTimeStamp().");
  double offsetX = mitk::IGTTimeStamp::GetInstance()->GetOffset(tester2);
  MITK_TEST_CONDITION_REQUIRED(offsetX==-1.0,"..Testing method GetOffset().");

  //start-tracking sets the reference-time, timestamps are relative to this time
  mitk::IGTTimeStamp::GetInstance()->Start(tester);
  MITK_INFO << "first device has started tracking\n";

  //sleeps for 20 ms
  #if defined (WIN32) || defined (_WIN32)
  Sleep(20);
  #else
  usleep(20000);
  #endif

  MITK_INFO << "supposed to have waited 20ms \n";

  double time_elapsed, relative_time_elapsed;

  //gets time elapsed since start
  time_elapsed = mitk::IGTTimeStamp::GetInstance()->GetElapsed();

  MITK_INFO << "have actually waited : " << time_elapsed << "ms \n";

  //elapsed time is not allowed to be too far from 20 ms (+-10ms)-> debugging "destroys" the correct value of course
  MITK_TEST_CONDITION_REQUIRED((time_elapsed-20) > -10 , "Testing if elapsed time is correct (-10)");
  //MITK_TEST_CONDITION_REQUIRED((time_elapsed-20) < 10 , "Testing if elapsed time is correct (+10)");

  //second "device" starts tracking
  mitk::IGTTimeStamp::GetInstance()->Start(tester2);
  MITK_INFO << "second device has started\n";
  //first device stops
  mitk::IGTTimeStamp::GetInstance()->Stop(tester);
  MITK_INFO << "first device has stopped tracking\n";

  time_elapsed = mitk::IGTTimeStamp::GetInstance()->GetElapsed();
  relative_time_elapsed = mitk::IGTTimeStamp::GetInstance()->GetElapsed(tester2);

  MITK_INFO << "time elapsed supposed to be greater than 20 ms\n";
  MITK_INFO << time_elapsed << " actually elapsed\n";

  MITK_INFO << "relative time elapsed supposed to be smaller than absolute time elapsed : \n";
  MITK_INFO << relative_time_elapsed << " actually elapsed\n";

  //relative timespan must be smaller than absolute timespan
  MITK_TEST_CONDITION_REQUIRED( time_elapsed > relative_time_elapsed , " testing if relative timespan is shorter than absolute timespan");

  //timestamp still has to be valid (tester2 still tracking), and has to be larger than 20ms
  //MITK_TEST_CONDITION_REQUIRED( time_elapsed > 15 , "testing if second device is still keeping the TimeStamp \"alive\"");

  mitk::IGTTimeStamp::GetInstance()->Stop(tester2);
  MITK_INFO << " second device has stopped tracking\n";
  time_elapsed = mitk::IGTTimeStamp::GetInstance()->GetElapsed();

  //when all devices have stopped, -1 has to be returned
  MITK_TEST_CONDITION_REQUIRED( time_elapsed == -1 , "testing if -1 is returned after all devices have stopped");

  //test set realtime clock
  mitk::RealTimeClock::Pointer myRealTimeClock = mitk::RealTimeClock::New();
  mitk::IGTTimeStamp::GetInstance()->SetRealTimeClock(myRealTimeClock);

  // always end with this!
  MITK_TEST_END();

}


