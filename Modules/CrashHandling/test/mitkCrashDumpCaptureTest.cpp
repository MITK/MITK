/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCrashDumpDatabase.h>
#include <mitkCrashDumpFacility.h>

#include "mitkCrashHandlingTestConfig.h"

#include <mitkIOUtil.h>
#include <mitkLog.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itksys/Process.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>

namespace
{
  struct HelperResult
  {
    bool Exited = false; // terminated normally, as opposed to a crash
    int ExitValue = -1;
    bool TimedOut = false;
  };

  HelperResult RunHelper(const std::string& mode, const std::filesystem::path& databaseDirectory)
  {
    const std::string helperPath =
      std::string(MITK_CRASHDUMP_TEST_HELPER_DIR) + "/" + MITK_CRASHDUMP_TEST_HELPER_EXECUTABLE;
    const std::string databaseString = databaseDirectory.string();
    const char* command[] = { helperPath.c_str(), mode.c_str(), databaseString.c_str(), nullptr };

    auto* process = itksysProcess_New();
    itksysProcess_SetCommand(process, command);
    itksysProcess_SetPipeShared(process, itksysProcess_Pipe_STDOUT, 1);
    itksysProcess_SetPipeShared(process, itksysProcess_Pipe_STDERR, 1);
    itksysProcess_SetTimeout(process, 60.0);
    itksysProcess_Execute(process);
    itksysProcess_WaitForExit(process, nullptr);

    HelperResult result;
    const auto state = static_cast<itksysProcess_State_e>(itksysProcess_GetState(process));
    result.Exited = state == itksysProcess_State_Exited;
    result.TimedOut = state == itksysProcess_State_Expired;

    if (result.Exited)
      result.ExitValue = itksysProcess_GetExitValue(process);

    itksysProcess_Delete(process);
    return result;
  }

  /** The out-of-process handler finishes writing after the crashed helper
   *  is already gone, so give it time before reading the database. */
  std::vector<mitk::CrashDumpInfo> WaitForDumps(const std::filesystem::path& databaseDirectory,
    std::size_t expectedCount)
  {
    for (int i = 0; i < 100; ++i)
    {
      auto dumps = mitk::ScanCrashDumps(databaseDirectory);
      if (dumps.size() >= expectedCount)
        return dumps;

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return mitk::ScanCrashDumps(databaseDirectory);
  }
}

class mitkCrashDumpCaptureTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCrashDumpCaptureTestSuite);
  MITK_TEST(CleanRunLeavesNoDump);
  MITK_TEST(CrashBySegvLeavesDump);
  MITK_TEST(CrashByAbortLeavesDump);
  MITK_TEST(CrashByStackOverflowLeavesDump);
  MITK_TEST(DumpsSurviveReinitialization);
  MITK_TEST(FacilityQueryAcknowledgeDeleteCycle);
  // On-demand and watchdog snapshots rely on Crashpad's DumpWithoutCrash,
  // which its macOS client does not provide.
#ifndef __APPLE__
  MITK_TEST(OnDemandSnapshotIsCapturedButNotSurfaced);
  MITK_TEST(HardKilledFreezeLeavesProvisionalDump);
  MITK_TEST(RecoveredFreezeLeavesNoDump);
#endif
  CPPUNIT_TEST_SUITE_END();

  std::filesystem::path m_DatabaseDirectory;

public:
  void setUp() override
  {
    m_DatabaseDirectory = mitk::IOUtil::CreateTemporaryDirectory("mitkCrashDumpCaptureTest_XXXXXX");
  }

  void tearDown() override
  {
    // Idempotent; guards against a test that armed the process-global
    // facility and then failed an assertion before shutting it down.
    mitk::CrashDumpFacility::Shutdown();

    std::error_code error;
    std::filesystem::remove_all(m_DatabaseDirectory, error);
  }

  /** Arming can legitimately be impossible in restricted environments
   *  (e.g. ptrace limits in containers), but only an explicitly allowlisted
   *  venue may downgrade that to a skip; everywhere else it is a failure. */
  [[noreturn]] void FailOrSkipUnarmedHelper()
  {
    if (std::getenv("MITK_CRASHTEST_ALLOW_SKIP") != nullptr)
    {
      MITK_INFO << "Crash-dump facility could not arm; skipping (MITK_CRASHTEST_ALLOW_SKIP is set).";
      std::exit(77); // ctest SKIP_RETURN_CODE
    }

    CPPUNIT_FAIL("Crash-dump facility failed to arm in the helper process "
                 "(and MITK_CRASHTEST_ALLOW_SKIP is not set).");
    std::abort(); // unreachable; CPPUNIT_FAIL throws
  }

  void RunCrashModeAndExpectOneDump(const std::string& mode)
  {
    const auto result = RunHelper(mode, m_DatabaseDirectory);

    if (result.Exited && result.ExitValue == 77)
      this->FailOrSkipUnarmedHelper();

    CPPUNIT_ASSERT_MESSAGE("helper must not time out", !result.TimedOut);
    CPPUNIT_ASSERT_MESSAGE("helper must not exit cleanly in mode " + mode,
      !(result.Exited && result.ExitValue == EXIT_SUCCESS));

    const auto dumps = WaitForDumps(m_DatabaseDirectory, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("expected exactly one dump for mode " + mode,
      std::size_t(1), dumps.size());
    CPPUNIT_ASSERT(dumps.front().SizeInBytes > 0);
  }

  void CleanRunLeavesNoDump()
  {
    const auto result = RunHelper("noop", m_DatabaseDirectory);

    if (result.Exited && result.ExitValue == 77)
      this->FailOrSkipUnarmedHelper();

    CPPUNIT_ASSERT(result.Exited);
    CPPUNIT_ASSERT_EQUAL(EXIT_SUCCESS, result.ExitValue);
    CPPUNIT_ASSERT(mitk::ScanCrashDumps(m_DatabaseDirectory).empty());
  }

  void CrashBySegvLeavesDump()
  {
    this->RunCrashModeAndExpectOneDump("segv");
  }

  void CrashByAbortLeavesDump()
  {
    this->RunCrashModeAndExpectOneDump("abort");
  }

  void CrashByStackOverflowLeavesDump()
  {
#ifdef _WIN32
    this->RunCrashModeAndExpectOneDump("stackoverflow");
#else
    // Alternate-signal-stack behavior differs across POSIX environments;
    // a missing dump is tolerated here, a produced one is still checked.
    const auto result = RunHelper("stackoverflow", m_DatabaseDirectory);

    if (result.Exited && result.ExitValue == 77)
      this->FailOrSkipUnarmedHelper();

    const auto dumps = WaitForDumps(m_DatabaseDirectory, 1);
    if (dumps.empty())
    {
      MITK_INFO << "No dump for stack overflow; tolerated on this platform.";
      return;
    }

    CPPUNIT_ASSERT(dumps.front().SizeInBytes > 0);
#endif
  }

  void DumpsSurviveReinitialization()
  {
    this->RunCrashModeAndExpectOneDump("segv");
    const auto dumpPath = mitk::ScanCrashDumps(m_DatabaseDirectory).front().Path;

    const auto rerun = RunHelper("noop", m_DatabaseDirectory);
    CPPUNIT_ASSERT(rerun.Exited);
    CPPUNIT_ASSERT_EQUAL(EXIT_SUCCESS, rerun.ExitValue);

    const auto dumps = mitk::ScanCrashDumps(m_DatabaseDirectory);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("re-initialization must not consume existing dumps",
      std::size_t(1), dumps.size());
    CPPUNIT_ASSERT(dumps.front().Path == dumpPath);
  }

  /** Headless end-to-end check of the facility operations the next-start
   *  dialog drives: after a real crash produced a dump, arm the facility in
   *  this process against the same database and walk list -> acknowledge ->
   *  delete without any UI. */
  void FacilityQueryAcknowledgeDeleteCycle()
  {
    this->RunCrashModeAndExpectOneDump("segv");

    mitk::CrashDumpFacility::Config config;
    config.DatabaseDirectory = m_DatabaseDirectory;
    config.ApplicationName = "mitkCrashDumpCaptureTest";
    config.ApplicationVersion = "1.0";

    if (!mitk::CrashDumpFacility::Initialize(config))
      this->FailOrSkipUnarmedHelper();

    CPPUNIT_ASSERT(mitk::CrashDumpFacility::IsActive());
    CPPUNIT_ASSERT(m_DatabaseDirectory == mitk::CrashDumpFacility::GetDatabaseDirectory());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::CrashDumpFacility::ListDumps().size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::CrashDumpFacility::ListUnacknowledgedDumps().size());

    // Acknowledge (what the dialog does once the user has seen the dump):
    // it stays on disk but is no longer offered on the next start.
    mitk::CrashDumpFacility::ClearCrashedLastRun();
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListUnacknowledgedDumps().empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::CrashDumpFacility::ListDumps().size());

    // Discard removes it from disk.
    const auto dumpPath = mitk::CrashDumpFacility::ListDumps().front().Path;
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::DeleteDump(dumpPath));
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListDumps().empty());

    mitk::CrashDumpFacility::Shutdown();
    CPPUNIT_ASSERT(!mitk::CrashDumpFacility::IsActive());
  }

#ifndef __APPLE__
  /** On-demand snapshot: the helper captures one without crashing (exits
   *  cleanly) and it is filed where the next-start dialog never surfaces it. */
  void OnDemandSnapshotIsCapturedButNotSurfaced()
  {
    const auto result = RunHelper("snapshot", m_DatabaseDirectory);

    if (result.Exited && result.ExitValue == 77)
      this->FailOrSkipUnarmedHelper();

    CPPUNIT_ASSERT_MESSAGE("snapshot helper must exit cleanly, without crashing",
      result.Exited && result.ExitValue == EXIT_SUCCESS);

    // The snapshot was written...
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::ScanCrashDumps(m_DatabaseDirectory).size());
    // ...into the on-demand subdirectory, so it is excluded from the
    // surfacable set and never triggers the next-start dialog.
    CPPUNIT_ASSERT(mitk::ScanCrashDumps(m_DatabaseDirectory, { "mitk-snapshots" }).empty());

    // The facility's typed view of the same facts (what the crash-test
    // plugin's dump list consumes).
    mitk::CrashDumpFacility::Config config;
    config.DatabaseDirectory = m_DatabaseDirectory;
    config.ApplicationName = "mitkCrashDumpCaptureTest";
    config.ApplicationVersion = "1.0";

    if (!mitk::CrashDumpFacility::Initialize(config))
      this->FailOrSkipUnarmedHelper();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1),
      mitk::CrashDumpFacility::ListSnapshots(mitk::SnapshotKind::OnDemand).size());
    CPPUNIT_ASSERT(
      mitk::CrashDumpFacility::ListSnapshots(mitk::SnapshotKind::WatchdogProvisional).empty());
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListDumps().empty());

    mitk::CrashDumpFacility::Shutdown();
  }

  /** UI-freeze watchdog, false-positive guard (direction A): a freeze that
   *  ends in a hard kill (no purge) leaves the provisional dump behind. */
  void HardKilledFreezeLeavesProvisionalDump()
  {
    const auto result = RunHelper("freeze", m_DatabaseDirectory);

    if (result.Exited && result.ExitValue == 77)
      this->FailOrSkipUnarmedHelper();

    CPPUNIT_ASSERT_MESSAGE("freeze helper must exit cleanly",
      result.Exited && result.ExitValue == EXIT_SUCCESS);

    CPPUNIT_ASSERT_MESSAGE("a hard-killed freeze must leave a provisional dump",
      !mitk::ScanCrashDumps(m_DatabaseDirectory).empty());
  }

  /** UI-freeze watchdog, false-positive guard (direction B): a freeze that
   *  recovers purges its provisional dump, leaving nothing to surface. */
  void RecoveredFreezeLeavesNoDump()
  {
    const auto result = RunHelper("freeze-recover", m_DatabaseDirectory);

    if (result.Exited && result.ExitValue == 77)
      this->FailOrSkipUnarmedHelper();

    CPPUNIT_ASSERT_MESSAGE("freeze-recover helper must exit cleanly",
      result.Exited && result.ExitValue == EXIT_SUCCESS);

    CPPUNIT_ASSERT_MESSAGE("a recovered freeze must leave no dump",
      mitk::ScanCrashDumps(m_DatabaseDirectory).empty());
  }
#endif
};

MITK_TEST_SUITE_REGISTRATION(mitkCrashDumpCapture)
