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

#include <QCoreApplication>
#include <QDir>
#include <QDateTime>

QString CreateEmptyTestFolder()
{
  QString dirname = QString("mitkBreakpadCrashReportingDumpTest-%1").arg(
    QDateTime::currentDateTime().toString( Qt::ISODate ).remove(':').remove('-').remove('+') );

  if ( QDir::temp().mkdir( dirname ) )
  {
    return QDir::tempPath() + QDir::separator() + dirname;
  }
  else
  {
    return QString::null;
  }
}

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

  QCoreApplication qtApplication(argc,argv);

  QString emptyTempFolder = CreateEmptyTestFolder();

  MITK_TEST_OUTPUT( << "Dumping files to " << qPrintable( emptyTempFolder ) );
  mitk::BreakpadCrashReporting crashReporting(emptyTempFolder);

  // start out-of-process crash dump server
  MITK_TEST_CONDITION_REQUIRED( crashReporting.StartCrashServer(true) == true, "Start out-of-process crash reporting server");

  // in-process reporting client (minimal code to tell other process to dump information)
  crashReporting.InitializeClientHandler(true);

  MITK_TEST_CONDITION_REQUIRED( true, "Start crash reporting client (in crashing process)");

  // provoke a seg-fault to make test crash
  crashReporting.CrashAppForTestPurpose();

  // always end with this!
  MITK_TEST_END()
}
