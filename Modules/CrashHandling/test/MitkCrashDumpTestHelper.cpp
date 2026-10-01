/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCrashDumpFacility.h>
#include <mitkHeartbeatMonitor.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

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
    std::cerr << "Usage: MitkCrashDumpTestHelper "
                 "<noop|segv|abort|stackoverflow|snapshot|snapshot-twice|freeze|freeze-recover> "
                 "<database-dir>" << std::endl;
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

  // Set after arming, as the applications do, so that the tests can tell
  // that the handler reads the run info at capture time.
  mitk::CrashDumpFacility::SetSessionLogFile(config.DatabaseDirectory / "helper-session.log");

  if (mode == "noop")
  {
    mitk::CrashDumpFacility::Shutdown();
    return EXIT_SUCCESS;
  }

  if (mode == "snapshot")
  {
    const auto path = mitk::CrashDumpFacility::CaptureSnapshot(mitk::SnapshotKind::OnDemand);
    mitk::CrashDumpFacility::Shutdown();

    if (!path.has_value())
    {
      std::cerr << "On-demand snapshot capture failed." << std::endl;
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  if (mode == "snapshot-twice")
  {
    const auto first = mitk::CrashDumpFacility::CaptureSnapshot(mitk::SnapshotKind::OnDemand);
    const auto second = mitk::CrashDumpFacility::CaptureSnapshot(mitk::SnapshotKind::OnDemand);
    mitk::CrashDumpFacility::Shutdown();

    if (!first.has_value() || !second.has_value())
    {
      std::cerr << "A session must be able to capture more than one snapshot." << std::endl;
      return EXIT_FAILURE;
    }

    if (*first == *second)
    {
      std::cerr << "Both snapshots were filed as '" << first->string() << "'." << std::endl;
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  if (mode == "freeze" || mode == "freeze-recover")
  {
    mitk::HeartbeatMonitor::Config monitorConfig;
    monitorConfig.Timeout = std::chrono::milliseconds(200);
    monitorConfig.CaptureInterval = std::chrono::milliseconds(100);
    monitorConfig.PollInterval = std::chrono::milliseconds(30);
    monitorConfig.MaxCapturesPerEpisode = 1;

    mitk::HeartbeatMonitor monitor(monitorConfig,
      [] { mitk::CrashDumpFacility::CaptureSnapshot(mitk::SnapshotKind::WatchdogProvisional); },
      [] { mitk::CrashDumpFacility::PurgeProvisionalSnapshots(); });
    monitor.Start();

    // Report in once to arm stall detection, then simulate a freeze by never
    // beating again; wait for the provisional dump.
    monitor.Beat();

    bool captured = false;
    for (int i = 0; i < 100 && !captured; ++i)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      captured = !mitk::CrashDumpFacility::ListProvisionalSnapshotsOfThisSession().empty();
    }

    if (!captured)
    {
      std::cerr << "Watchdog did not produce a provisional dump." << std::endl;
      return EXIT_FAILURE;
    }

    if (mode == "freeze")
    {
      // Simulate a hard kill mid-freeze: stop the monitor but do NOT shut the
      // facility down, so no purge runs and the provisional dump persists.
      monitor.Stop();
      return EXIT_SUCCESS;
    }

    // freeze-recover: resume beating so the recovery callback purges the dump.
    bool recovered = false;
    for (int i = 0; i < 100 && !recovered; ++i)
    {
      monitor.Beat();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      recovered = mitk::CrashDumpFacility::ListProvisionalSnapshotsOfThisSession().empty();
    }
    monitor.Stop();
    mitk::CrashDumpFacility::Shutdown();

    return recovered ? EXIT_SUCCESS : EXIT_FAILURE;
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
