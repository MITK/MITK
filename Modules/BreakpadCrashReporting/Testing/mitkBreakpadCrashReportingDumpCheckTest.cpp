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

#include <QDir>
#include <QTime>
#include <QCoreApplication>

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
  QTime dieTime = QTime::currentTime().addSecs(3);
  while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
#endif

  QString dirnamePattern = QString("mitkBreakpadCrashReportingDumpTest-*");

  QDir tempDir = QDir::temp();
  QStringList nameFilters;
  nameFilters << dirnamePattern;
  tempDir.setNameFilters( nameFilters );
  tempDir.setSorting( QDir::Time );

  QStringList dumpFolders = tempDir.entryList();

  MITK_TEST_CONDITION_REQUIRED( !dumpFolders.empty(), "Found at least one folder matching mitkBreakpadCrashReportingDumpTest-*")

  QString foldernameToCheck = dumpFolders.first(); // most recent

  QDir folderToCheck( QDir::tempPath() + QDir::separator() + foldernameToCheck );
  MITK_TEST_CONDITION( folderToCheck.exists(), qPrintable(foldernameToCheck) << " exists")
  QStringList dumpFiles = folderToCheck.entryList( QDir::Files );
  MITK_TEST_CONDITION( dumpFiles.size() == 1, qPrintable(foldernameToCheck) << " has one entry (the DUMP)")

  MITK_TEST_OUTPUT( << "----------------------------------------" )
  MITK_TEST_OUTPUT( << "Files in " << qPrintable(folderToCheck.path()) )
  foreach(QString f, dumpFiles)
  {
    MITK_TEST_OUTPUT( << " .. entry " << qPrintable(f) )
    folderToCheck.remove(f);
  }
  MITK_TEST_OUTPUT( << "--- End of file list--------------------" )

  MITK_TEST_CONDITION( tempDir.rmdir( foldernameToCheck ), "Clean up created folders, i.e. remove " << qPrintable(foldernameToCheck) )

  // always end with this!
  MITK_TEST_END()
}
