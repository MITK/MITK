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

#include "mitkBreakpadCrashReporting.h"

#include <mitkLog.h>
#include "mitkLogMacros.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  std::string folderForCrashDumps ="";

  if (argc != 2)
  {
    MITK_ERROR << "Exactly 2 argument expected, got " << argc << " instead!";
    exit(2);
  }
  else
  {
    folderForCrashDumps = argv[1];
  }

  mitk::BreakpadCrashReporting crashReporting( folderForCrashDumps );

  // start out-of-process crash dump server
  crashReporting.StartCrashServer(true);

  // in-process reporting client (minimal code to tell other process to dump information)
  crashReporting.InitializeClientHandler(true);

  // provoke a seg-fault to make test crash
  crashReporting.CrashAppForTestPurpose();
}
