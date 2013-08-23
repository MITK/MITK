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

#include "mitkTestingMacros.h"

#include "mitkBreakpadCrashReporting.h"
#include <itksys/SystemTools.hxx>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

std::string CreateEmptyTestFolder()
{
  std::string modulePath = mitk::BreakpadCrashReporting::GetModulePath(); // get path of executable

  char dateTime[15];
  time_t now = time(0);
  tm* time = localtime( &now );
  strftime( dateTime, 15, "%Y%m%d%H%M%S", time );

  std::string dirName = modulePath + "/breakpadtestdump/" + "mitkBreakpadCrashReportingDumpTest-" + dateTime;

  if( itksys::SystemTools::MakeDirectory(dirName.c_str()) )
  {
    return dirName;
  }
  return "";
};

//void SignalHandler( int signo, siginfo_t

/**
  \brief Start crash reporting and crash (expectedly).

  This method is excpected to setup BreakpadCrashReporting,
  then provoke a crash, thus creating a crash dump in a configured
  folder.

  CMake is configured to expect failure of this test.
  In addition we check the actual existence of a crash dump
  in mitkBreakpadCrashReportingDumpCheckTest.
*/
int mitkBreakpadCrashReportingDumpTest(int argc, char** const argv)
{
  // always start with this!
  MITK_TEST_BEGIN("mitkBreakpadCrashReportingDumpTest")

  std::string emptyTempFolder = CreateEmptyTestFolder();

  MITK_TEST_OUTPUT( << "Dumping files to " << emptyTempFolder );
  mitk::BreakpadCrashReporting crashReporting( emptyTempFolder );

  // start out-of-process crash dump server
  MITK_TEST_CONDITION_REQUIRED( crashReporting.StartCrashServer(true) == true, "Start out-of-process crash reporting server");

  // in-process reporting client (minimal code to tell other process to dump information)
  crashReporting.InitializeClientHandler(true);

  MITK_TEST_CONDITION_REQUIRED( true, "Start crash reporting client (in crashing process)");

  // provoke a seg-fault to make test crash -> call external application which will crash
#ifdef WIN32
  std::string commandline = mitk::BreakpadCrashReporting::GetModulePath() + "/BreakpadCrashReportingDumpTestApplication" + " " + emptyTempFolder;
  system( commandline.c_str() );
#elif __gnu_linux__
  // provoke a seg-fault to make test crash
  crashReporting.CrashAppForTestPurpose();
#endif

  // always end with this!
  MITK_TEST_END()
}
