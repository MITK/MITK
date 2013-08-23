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

#ifdef WIN32
  #include <windows.h>
  #include <direct.h>
#elif __gnu_linux__
  #include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>

/**
  \brief Checks result of crash test mitkBreakpadCrashReportingDumpTest.

  Checks for the most recent directory that would have been created by
  a crashing test mitkBreakpadCrashReportingDumpTest (identified by common
  string in both tests).

  The directory must contain exactly one file, which is expected to be
  the cash dump.
*/
int mitkBreakpadCrashReportingDumpCheckTest(int, char** const)
{
  // always start with this!
  MITK_TEST_BEGIN("mitkBreakpadCrashReportingDumpCheckTest");
#ifdef WIN32
  // Waiting a bit to let the crash reporting server do its work
  // This is not neccessary on Linux because we run the SERVER as
  // the unittest, so when it is done, the crash dump must exist.
  // (On Windows, we run the client and this in turn starts a server)
  Sleep( 3000 );
#endif
  std::string crashDumpFolderString = mitk::BreakpadCrashReporting::GetModulePath() + "/breakpadtestdump/";
  itk::Directory::Pointer crashDumpDirectory = itk::Directory::New();
  MITK_TEST_CONDITION_REQUIRED( crashDumpDirectory->Load( crashDumpFolderString.c_str() ), "Crash dump folder exists." );

  // since the folder is only used for this testing purposes, there should only be crash dump folders here
  // so we only have to check if the crash dump directory contains anything
  MITK_TEST_CONDITION_REQUIRED( crashDumpDirectory->GetNumberOfFiles() > 0, "Found at least one folder in crash dump directory.");
  for ( unsigned int i = 0; i < crashDumpDirectory->GetNumberOfFiles(); ++i )
  {
    std::string folderName = crashDumpFolderString + "/" + crashDumpDirectory->GetFile( i );
    if( folderName.find( "mitkBreakpadCrashReportingDumpTest-" ) == std::string::npos )
    {
      // not a crash pad folder
      continue;
    }

    itk::Directory::Pointer folder = itk::Directory::New();
    MITK_TEST_CONDITION( folder->Load( folderName.c_str() ), "Directory " << folderName << " could be opened.");
    bool dmpFileFound = false;
    for ( unsigned int j = 0; j < folder->GetNumberOfFiles(); ++j )
    {
      std::string filename = folderName + "/" + folder->GetFile( j );

      std::string extension = itksys::SystemTools::GetFilenameExtension(filename.c_str());

      if(extension.compare(".dmp")==0)
      {
        dmpFileFound = true;
      }
      remove( filename.c_str() );
    }
    MITK_TEST_CONDITION( dmpFileFound, "Dump file was created." );
    rmdir( folderName.c_str() );
  }
  rmdir( crashDumpFolderString.c_str() );

  // always end with this!
  MITK_TEST_END()
}
