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
#include <itkDirectory.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>

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

/**
  \brief Start crash reporting and crash (expectedly).

  This method is excpected to setup BreakpadCrashReporting,
  then provoke a crash, thus creating a crash dump in a configured
  folder. Then we check the actual existence of the crash dump and if it has any content.
  If the dump generation was not successful the dump file will be empty or not existent.

  In order to get the output of ctest correctly we provoke the
  crash in a different process, else the test itself would crash.

Linux:
  The test is currently deactivated for Linux.
  ToDo:
  Currently there seems to be a problem in linux with crash dump generation. In this test we
  provoke a segfault. Sometimes the crash dump generation gets in a state where it waits for
  the child process (the process which crashed) to change the process state, but the child
  process is allready attach via ptrace to the parent process and therefore has allready the state
  stopped and does not change until it gets a signal from the parent process. The parent process
  itself waits for the child process to change the state.
*/
int mitkBreakpadCrashReportingDumpTest(int argc, char** const argv)
{
  // always start with this!
  MITK_TEST_BEGIN("mitkBreakpadCrashReportingDumpTest")

  std::string emptyTempFolder = CreateEmptyTestFolder();

  MITK_TEST_OUTPUT( << "Dumping files to " << emptyTempFolder );
#ifdef WIN32
  mitk::BreakpadCrashReporting crashReporting( emptyTempFolder );

  // start out-of-process crash dump server
  MITK_TEST_CONDITION_REQUIRED( crashReporting.StartCrashServer(true) == true, "Start out-of-process crash reporting server");

  // in-process reporting client (minimal code to tell other process to dump information)
  crashReporting.InitializeClientHandler(true);

  MITK_TEST_CONDITION_REQUIRED( true, "Start crash reporting client (in crashing process)");

  // provoke a seg-fault to make test crash -> call external application which will crash
  std::string commandline = mitk::BreakpadCrashReporting::GetModulePath() + "/BreakpadCrashReportingDumpTestApplication" + " " + emptyTempFolder;
  system( commandline.c_str() );
#elif __gnu_linux__
  pid_t child_pid = fork();

  if ( child_pid != 0)
  {
    waitpid(child_pid, NULL, __WALL);
    /*
    if( waitpid(child_pid, NULL, __WALL) != child_pid ) {
      // the child process does not complete within 30 seconds
      kill(child_pid, SIGTERM);
    }
    */
  }
  else
  {
    //alarm(5);
    mitk::BreakpadCrashReporting crashReporting( emptyTempFolder );

    // start out-of-process crash dump server
    crashReporting.StartCrashServer(true);

    // in-process reporting client (minimal code to tell other process to dump information)
    crashReporting.InitializeClientHandler(true);

    // provoke a seg-fault to make test crash
    crashReporting.CrashAppForTestPurpose();
  }
#endif

  // check dump creation
#ifdef WIN32
  // Waiting a bit to let the crash reporting server do its work
  // This is not neccessary on Linux because we run the SERVER as
  // the unittest, so when it is done, the crash dump must exist.
  // (On Windows, we run the client and this in turn starts a server)
  Sleep( 3000 );
#endif
  itk::Directory::Pointer crashDumpDirectory = itk::Directory::New();
  MITK_TEST_CONDITION_REQUIRED( crashDumpDirectory->Load( emptyTempFolder.c_str() ), "Crash dump folder exists." );

  // since the folder is only used for this testing purposes, there should only be crash dump folders here
  // so we only have to check if the crash dump directory contains anything
  MITK_TEST_CONDITION_REQUIRED( crashDumpDirectory->GetNumberOfFiles() > 0, "Found at least one file in crash dump directory.");
  bool dmpFileFound = false;
  for ( unsigned int i = 0; i < crashDumpDirectory->GetNumberOfFiles(); ++i )
  {
    std::string filename = emptyTempFolder + "/" + crashDumpDirectory->GetFile( i );

    std::string extension = itksys::SystemTools::GetFilenameExtension(filename.c_str());

    if(extension.compare(".dmp")==0)
    {
      dmpFileFound = true;
      std::ifstream in(filename.c_str(), std::ifstream::in | std::ifstream::binary);
      in.seekg(0, std::ifstream::end);
      MITK_TEST_CONDITION( in.tellg() > 0, "The dump file is not empty." );
    }
    remove( filename.c_str() );
  }
  MITK_TEST_CONDITION( dmpFileFound, "Dump file was created." );
  rmdir( emptyTempFolder.c_str() );

  // always end with this!
  MITK_TEST_END()
}
