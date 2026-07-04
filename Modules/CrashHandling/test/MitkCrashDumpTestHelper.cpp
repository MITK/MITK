/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCrashDumpFacility.h>

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
  /* Always zero, but the compiler cannot prove it: the early return keeps
     MSVC's C4717 (unconditional recursion) quiet and the frame usage keeps
     the recursion from being optimized into a loop. */
  volatile std::size_t s_StopDepth = 0;

  int RecurseUntilStackOverflow(std::size_t depth)
  {
    volatile char frame[4096];
    frame[0] = static_cast<char>(depth);

    if (depth == s_StopDepth)
      return frame[0];

    return RecurseUntilStackOverflow(depth + 1) + frame[0];
  }
}

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    std::cerr << "Usage: MitkCrashDumpTestHelper <noop|segv|abort|stackoverflow> <database-dir>" << std::endl;
    return EXIT_FAILURE;
  }

  const std::string mode = argv[1];

  mitk::CrashDumpFacility::Config config;
  config.DatabaseDirectory = argv[2];
  config.ApplicationName = "MitkCrashDumpTestHelper";
  config.ApplicationVersion = "1.0";

  if (!mitk::CrashDumpFacility::Initialize(config))
  {
    std::cerr << "Crash-dump facility failed to arm." << std::endl;
    return 77; // the launching test decides whether skipping is allowed
  }

  if (mode == "noop")
  {
    mitk::CrashDumpFacility::Shutdown();
    return EXIT_SUCCESS;
  }

  if (mode == "segv")
  {
    int* volatile nullPointer = nullptr;
    *nullPointer = 42;
  }
  else if (mode == "abort")
  {
    std::abort();
  }
  else if (mode == "stackoverflow")
  {
    return RecurseUntilStackOverflow(1);
  }
  else
  {
    std::cerr << "Unknown mode '" << mode << "'." << std::endl;
    return EXIT_FAILURE;
  }

  std::cerr << "Mode '" << mode << "' did not crash the process." << std::endl;
  return EXIT_FAILURE;
}
