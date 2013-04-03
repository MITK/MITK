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

int mitkBreakpadCrashReportingDumpTest(int argc, char** argv)
{
  // always start with this!
  MITK_TEST_BEGIN("mitkBreakpadCrashReportingDumpTest")

  QCoreApplication qtApplication(argc,argv);

  mitk::BreakpadCrashReporting crashReporting;

  // start out-of-process crash dump server
  MITK_TEST_CONDITION_REQUIRED( crashReporting.StartCrashServer(true) == true, "Start out-of-process crash reporting server");

  // in-process reporting client (minimal code to tell other process to dump information)
  crashReporting.InitializeClientHandler(true);

  MITK_TEST_CONDITION_REQUIRED( true, "Start crash reporting client (in crashing process)");

  // provoke a seg-fault to make test crash
  crashReporting.CrashAppForTestPurpose();

  MITK_TEST_CONDITION_REQUIRED( false, "Test failed, did not crash...)");

  // always end with this!
  MITK_TEST_END()
}
