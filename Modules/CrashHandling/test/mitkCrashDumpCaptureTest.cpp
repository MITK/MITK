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
#include <fstream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

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

  // Only the snapshot and delete tests inspect Crashpad's bookkeeping, and
  // those are excluded on macOS.
#ifndef __APPLE__
  /** Crashpad's bookkeeping for the report that produced \p dumpPath: the
   *  sibling metadata file and the report's attachment directory. Both are
   *  keyed by the report UUID, which is the dump's stem as Crashpad wrote it -
   *  so this only answers for a dump still under the name Crashpad gave it.
   *  For one MITK has filed, use AnyReportResidueExists(). */
  bool ReportResidueExists(const std::filesystem::path& databaseDirectory,
    const std::filesystem::path& dumpPath)
  {
    const auto uuid = dumpPath.stem();

    return std::filesystem::exists(std::filesystem::path(dumpPath).replace_extension(".meta"))
        || std::filesystem::exists(databaseDirectory / "attachments" / uuid);
  }

  /** Whether any Crashpad report record is left anywhere in the database,
   *  without assuming a UUID: filing a snapshot renames it, so its own path no
   *  longer names the report it came from. */
  bool AnyReportResidueExists(const std::filesystem::path& databaseDirectory)
  {
    const auto attachments = databaseDirectory / "attachments";

    if (std::filesystem::exists(attachments) && !std::filesystem::is_empty(attachments))
      return true;

    std::error_code error;
    for (const auto& entry :
      std::filesystem::recursive_directory_iterator(databaseDirectory,
        std::filesystem::directory_options::skip_permission_denied, error))
    {
      if (entry.is_regular_file(error) && entry.path().extension() == ".meta")
        return true;
    }

    return false;
  }
#endif

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
  MITK_TEST(UnarmedFacilityStillListsAndDeletes);
  MITK_TEST(AcknowledgingLeavesDumpsWrittenMeanwhile);
  MITK_TEST(SettingsDisableArming);
  MITK_TEST(RetentionFollowsSettings);
  // On-demand and watchdog snapshots rely on Crashpad's DumpWithoutCrash,
  // which its macOS client does not provide.
#ifndef __APPLE__
  MITK_TEST(OnDemandSnapshotIsCapturedButNotSurfaced);
  MITK_TEST(RepeatedSnapshotsAreAllCaptured);
  MITK_TEST(DeleteDumpLeavesNoReportResidue);
  MITK_TEST(HardKilledFreezeLeavesProvisionalDump);
  MITK_TEST(RecoveredFreezeLeavesNoDump);
  MITK_TEST(ListAllDumpsClassifiesEveryKind);
  MITK_TEST(ProvisionalSnapshotsOfThisSessionAreNotListed);
  MITK_TEST(ProvisionalCapturesDoNotEvictSurvivors);
  MITK_TEST(SnapshotCarriesRunInfo);
  // Run-info attachments, which crash dumps rely on, are verified on Windows
  // only so far; Linux shares the code path and is expected to pass, macOS
  // is unexplored.
  MITK_TEST(CrashDumpCarriesRunInfo);
  MITK_TEST(CrashDumpDoesNotKeepALaterSessionsLog);
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

  mitk::CrashDumpFacility::Config MakeConfig(bool arm = true) const
  {
    mitk::CrashDumpFacility::Config config;
    config.DatabaseDirectory = m_DatabaseDirectory;
    config.ApplicationName = "mitkCrashDumpCaptureTest";
    config.ApplicationVersion = "1.0";
    config.Arm = arm;
    return config;
  }

  std::filesystem::path CreateFakeDump(const std::string& relativePath, int ageInSeconds) const
  {
    const auto path = m_DatabaseDirectory / relativePath;

    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path) << "minidump placeholder";
    std::filesystem::last_write_time(path,
      std::filesystem::file_time_type::clock::now() - std::chrono::seconds(ageInSeconds));

    return path;
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
    mitk::CrashDumpFacility::ClearCrashedLastRun(mitk::CrashDumpFacility::ListUnacknowledgedDumps());
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListUnacknowledgedDumps().empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::CrashDumpFacility::ListDumps().size());

    // Discard removes it from disk.
    const auto dumpPath = mitk::CrashDumpFacility::ListDumps().front().Path;
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::DeleteDump(dumpPath));
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListDumps().empty());

    mitk::CrashDumpFacility::Shutdown();
    CPPUNIT_ASSERT(!mitk::CrashDumpFacility::IsActive());
  }

  /** Crash dumps disabled for a session (environment variable or
   *  command-line flag) must not take the dumps already on disk out of the
   *  user's reach. */
  void UnarmedFacilityStillListsAndDeletes()
  {
    const auto dump = this->CreateFakeDump("reports/earlier.dmp", 60);

    CPPUNIT_ASSERT_MESSAGE("Config::Arm=false must not arm",
      !mitk::CrashDumpFacility::Initialize(this->MakeConfig(false)));
    CPPUNIT_ASSERT(!mitk::CrashDumpFacility::IsActive());
    CPPUNIT_ASSERT(m_DatabaseDirectory == mitk::CrashDumpFacility::GetDatabaseDirectory());

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::CrashDumpFacility::ListAllDumps().size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::CrashDumpFacility::ListUnacknowledgedDumps().size());

    CPPUNIT_ASSERT(mitk::CrashDumpFacility::DeleteDump(dump));
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListAllDumps().empty());
  }

  /** The dialog may stay open while another instance crashes: acknowledging
   *  what was shown must leave that newer dump to surface on the next start. */
  void AcknowledgingLeavesDumpsWrittenMeanwhile()
  {
    const auto dumpA = this->CreateFakeDump("reports/shown.dmp", 120);

    CPPUNIT_ASSERT(!mitk::CrashDumpFacility::Initialize(this->MakeConfig(false)));

    const auto shown = mitk::CrashDumpFacility::ListUnacknowledgedDumps();
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), shown.size());
    CPPUNIT_ASSERT(dumpA == shown.front().Path);
    CPPUNIT_ASSERT(shown.front().Unacknowledged);
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListAllDumps().front().Unacknowledged);

    const auto dumpB = this->CreateFakeDump("reports/written-meanwhile.dmp", 30);

    mitk::CrashDumpFacility::ClearCrashedLastRun(shown);

    const auto remaining = mitk::CrashDumpFacility::ListUnacknowledgedDumps();
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), remaining.size());
    CPPUNIT_ASSERT(dumpB == remaining.front().Path);

    for (const auto& dump : mitk::CrashDumpFacility::ListAllDumps())
      CPPUNIT_ASSERT_EQUAL(dumpB == dump.Path, dump.Unacknowledged);
  }

  void SettingsDisableArming()
  {
    mitk::CrashDumpSettings settings;
    settings.Enabled = false;
    CPPUNIT_ASSERT(mitk::WriteCrashDumpSettings(m_DatabaseDirectory, settings));

    CPPUNIT_ASSERT_MESSAGE("enabled=false in the settings file must not arm",
      !mitk::CrashDumpFacility::Initialize(this->MakeConfig()));
    CPPUNIT_ASSERT(!mitk::CrashDumpFacility::IsActive());
    CPPUNIT_ASSERT(!mitk::CrashDumpFacility::GetSettings().Enabled);
    CPPUNIT_ASSERT(m_DatabaseDirectory == mitk::CrashDumpFacility::GetDatabaseDirectory());
  }

  void RetentionFollowsSettings()
  {
    const std::vector<std::string> areas = { "reports", "mitk-snapshots", "mitk-pending-freeze" };

    for (const auto& area : areas)
    {
      for (int i = 0; i < 5; ++i)
        this->CreateFakeDump(area + "/dump" + std::to_string(i) + ".dmp", 10 * (i + 1));
    }

    mitk::CrashDumpSettings settings;
    settings.MaxDumpsPerKind = 3;
    CPPUNIT_ASSERT(mitk::WriteCrashDumpSettings(m_DatabaseDirectory, settings));

    mitk::CrashDumpFacility::Initialize(this->MakeConfig(false));
    CPPUNIT_ASSERT_EQUAL(3, mitk::CrashDumpFacility::GetSettings().MaxDumpsPerKind);

    for (const auto& area : areas)
    {
      const auto dumps = mitk::ScanCrashDumps(m_DatabaseDirectory / area);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("dumps left in " + area, std::size_t(3), dumps.size());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("the newest survive in " + area,
        std::string("dump0.dmp"), dumps.front().Path.filename().string());
    }
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
    const auto dumps = mitk::ScanCrashDumps(m_DatabaseDirectory);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), dumps.size());
    // ...into the on-demand subdirectory, so it is excluded from the
    // surfacable set and never triggers the next-start dialog.
    CPPUNIT_ASSERT(mitk::ScanCrashDumps(m_DatabaseDirectory, { "mitk-snapshots" }).empty());

    // Filing the snapshot must free the report Crashpad wrote it as; an
    // orphaned record keeps the session's report ID claimed and blocks every
    // later capture. Filing renames the dump, so the report can only be found
    // by sweeping the database, not from the filed name.
    CPPUNIT_ASSERT_MESSAGE("a filed snapshot must leave no Crashpad report behind",
      !AnyReportResidueExists(m_DatabaseDirectory));

    // Only a dump Crashpad has finished writing may be filed, so the staging
    // directory is neither a source nor a leftover.
    CPPUNIT_ASSERT_MESSAGE("a snapshot must never be filed out of the staging directory",
      dumps.front().Path.string().find("/new/") == std::string::npos &&
      dumps.front().Path.string().find("\\new\\") == std::string::npos);
    CPPUNIT_ASSERT_MESSAGE("nothing may be left staged",
      !std::filesystem::exists(m_DatabaseDirectory / "new") ||
      std::filesystem::is_empty(m_DatabaseDirectory / "new"));

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

  /** Every capture in a session must be filed, under its own name: the
   *  watchdog's per-episode captures and the error dialog's "Capture
   *  diagnostics" button both take more than one snapshot per session. */
  void RepeatedSnapshotsAreAllCaptured()
  {
    const auto result = RunHelper("snapshot-twice", m_DatabaseDirectory);

    if (result.Exited && result.ExitValue == 77)
      this->FailOrSkipUnarmedHelper();

    CPPUNIT_ASSERT_MESSAGE("snapshot-twice helper must exit cleanly",
      result.Exited && result.ExitValue == EXIT_SUCCESS);

    const auto snapshots = mitk::ScanCrashDumps(m_DatabaseDirectory / "mitk-snapshots");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("two captures must leave two dumps",
      std::size_t(2), snapshots.size());
    CPPUNIT_ASSERT(snapshots[0].Path != snapshots[1].Path);
    CPPUNIT_ASSERT(snapshots[0].SizeInBytes > 0 && snapshots[1].SizeInBytes > 0);
  }

  /** Discarding a dump must take Crashpad's record of it along, as
   *  CrashDumpFacility::DeleteDump documents. */
  void DeleteDumpLeavesNoReportResidue()
  {
    this->RunCrashModeAndExpectOneDump("segv");

    mitk::CrashDumpFacility::Config config;
    config.DatabaseDirectory = m_DatabaseDirectory;
    config.ApplicationName = "mitkCrashDumpCaptureTest";
    config.ApplicationVersion = "1.0";

    if (!mitk::CrashDumpFacility::Initialize(config))
      this->FailOrSkipUnarmedHelper();

    const auto dumpPath = mitk::CrashDumpFacility::ListDumps().front().Path;
    CPPUNIT_ASSERT(ReportResidueExists(m_DatabaseDirectory, dumpPath));

    CPPUNIT_ASSERT(mitk::CrashDumpFacility::DeleteDump(dumpPath));
    CPPUNIT_ASSERT(!ReportResidueExists(m_DatabaseDirectory, dumpPath));

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

  void RunHelperExpectingCleanExit(const std::string& mode)
  {
    const auto result = RunHelper(mode, m_DatabaseDirectory);

    if (result.Exited && result.ExitValue == 77)
      this->FailOrSkipUnarmedHelper();

    CPPUNIT_ASSERT_MESSAGE(mode + " helper must exit cleanly",
      result.Exited && result.ExitValue == EXIT_SUCCESS);
  }

  void ListAllDumpsClassifiesEveryKind()
  {
    this->RunCrashModeAndExpectOneDump("segv");
    this->RunHelperExpectingCleanExit("snapshot");
    this->RunHelperExpectingCleanExit("freeze");

    mitk::CrashDumpFacility::Initialize(this->MakeConfig(false));
    const auto dumps = mitk::CrashDumpFacility::ListAllDumps();

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), dumps.size());
    CPPUNIT_ASSERT(mitk::DumpKind::UnresponsiveTerminated == dumps[0].Kind);
    CPPUNIT_ASSERT(mitk::DumpKind::OnDemand == dumps[1].Kind);
    CPPUNIT_ASSERT(mitk::DumpKind::Crash == dumps[2].Kind);
    CPPUNIT_ASSERT(dumps[0].LastWriteTime >= dumps[1].LastWriteTime);
    CPPUNIT_ASSERT(dumps[1].LastWriteTime >= dumps[2].LastWriteTime);
  }

  /** A provisional snapshot of the running session is deleted again when
   *  the freeze recovers; it must not be offered for handling, surface on the
   *  next start through the watermark, or count as already shown. */
  void ProvisionalSnapshotsOfThisSessionAreNotListed()
  {
    if (!mitk::CrashDumpFacility::Initialize(this->MakeConfig()))
      this->FailOrSkipUnarmedHelper();

    const auto snapshot = mitk::CrashDumpFacility::CaptureSnapshot(mitk::SnapshotKind::WatchdogProvisional);
    CPPUNIT_ASSERT(snapshot.has_value());

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::CrashDumpFacility::ListProvisionalSnapshotsOfThisSession().size());
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListAllDumps().empty());
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListDumps().empty());
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListUnacknowledgedDumps().empty());

    mitk::CrashDumpFacility::ClearCrashedLastRun(mitk::CrashDumpFacility::ListUnacknowledgedDumps());
    CPPUNIT_ASSERT_MESSAGE("a provisional snapshot must not move the watermark",
      !mitk::ReadLastAcknowledgedTime(m_DatabaseDirectory).has_value());

    mitk::CrashDumpFacility::PurgeProvisionalSnapshots();
    CPPUNIT_ASSERT(!std::filesystem::exists(*snapshot));
  }

  /** At the lowest retention, a freeze episode's captures must neither evict
   *  an earlier hard-killed freeze nor each other; after recovery the
   *  survivor has to be there for the next-start dialog. */
  void ProvisionalCapturesDoNotEvictSurvivors()
  {
    mitk::CrashDumpSettings settings;
    settings.MaxDumpsPerKind = mitk::CrashDumpSettings::MinRetention;
    CPPUNIT_ASSERT(mitk::WriteCrashDumpSettings(m_DatabaseDirectory, settings));

    const auto survivor = this->CreateFakeDump("mitk-pending-freeze/survivor.dmp", 60);

    if (!mitk::CrashDumpFacility::Initialize(this->MakeConfig()))
      this->FailOrSkipUnarmedHelper();

    const auto first = mitk::CrashDumpFacility::CaptureSnapshot(mitk::SnapshotKind::WatchdogProvisional);
    const auto second = mitk::CrashDumpFacility::CaptureSnapshot(mitk::SnapshotKind::WatchdogProvisional);
    CPPUNIT_ASSERT(first.has_value() && second.has_value());

    CPPUNIT_ASSERT_MESSAGE("a capture must not evict the survivor", std::filesystem::exists(survivor));
    CPPUNIT_ASSERT_MESSAGE("a capture must not evict the episode's earlier one", std::filesystem::exists(*first));

    mitk::CrashDumpFacility::PurgeProvisionalSnapshots();
    CPPUNIT_ASSERT(std::filesystem::exists(survivor));
    CPPUNIT_ASSERT(!std::filesystem::exists(*first));
    CPPUNIT_ASSERT(!std::filesystem::exists(*second));
  }

  std::filesystem::path HelperLog() const
  {
    return m_DatabaseDirectory / "helper-session.log";
  }

  static std::string ReadFile(const std::filesystem::path& file)
  {
    std::ifstream stream(file);
    return std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
  }

  void CheckHelperRunInfo(const mitk::CrashDumpInfo& dump)
  {
    CPPUNIT_ASSERT_MESSAGE("the dump must carry run info", dump.RunInfo.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("MitkCrashDumpTestHelper 1.0"), dump.RunInfo->Release);
    CPPUNIT_ASSERT(std::filesystem::equivalent(MITK_CRASHDUMP_TEST_HELPER_DIR, dump.RunInfo->InstallDirectory));
    CPPUNIT_ASSERT_MESSAGE("the log path set after arming must be captured",
      this->HelperLog() == dump.RunInfo->LogFile);
  }

  void CrashDumpCarriesRunInfo()
  {
    std::ofstream(this->HelperLog()) << "crashed session";
    this->RunCrashModeAndExpectOneDump("segv");

    // The next start of the crashed install initializes before the log is
    // rotated, which is when the log under the recorded name is still the
    // crashed session's.
    mitk::CrashDumpFacility::Initialize(this->MakeConfig(false));

    // Rotation then hands the name to the new session.
    std::ofstream(this->HelperLog()) << "next session";

    const auto dumps = mitk::CrashDumpFacility::ListAllDumps();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), dumps.size());
    this->CheckHelperRunInfo(dumps.front());
    CPPUNIT_ASSERT_MESSAGE("the dump must keep its own session's log",
      !dumps.front().SessionLog.empty());
    CPPUNIT_ASSERT_EQUAL(std::string("crashed session"), ReadFile(dumps.front().SessionLog));

    const auto sidecar = mitk::GetRunInfoSidecarPath(dumps.front().Path);
    const auto logCopy = dumps.front().SessionLog;
    CPPUNIT_ASSERT(std::filesystem::exists(sidecar));
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::DeleteDump(dumps.front().Path));
    CPPUNIT_ASSERT_MESSAGE("deleting a dump must take its run info along", !std::filesystem::exists(sidecar));
    CPPUNIT_ASSERT_MESSAGE("deleting a dump must take its log along", !std::filesystem::exists(logCopy));
  }

  /** If the log under the recorded name was written after the dump, the
   *  crashed install has started again and rotated its logs before the dump
   *  was first seen; that log is another session's and must not be kept. */
  void CrashDumpDoesNotKeepALaterSessionsLog()
  {
    this->RunCrashModeAndExpectOneDump("segv");

    const auto dumpTime = mitk::ScanCrashDumps(m_DatabaseDirectory).front().LastWriteTime;
    std::ofstream(this->HelperLog()) << "later session";
    std::filesystem::last_write_time(this->HelperLog(), dumpTime + std::chrono::minutes(5));

    mitk::CrashDumpFacility::Initialize(this->MakeConfig(false));
    const auto dumps = mitk::CrashDumpFacility::ListAllDumps();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), dumps.size());
    CPPUNIT_ASSERT(dumps.front().RunInfo.has_value());
    CPPUNIT_ASSERT(dumps.front().SessionLog.empty());
  }

  void SnapshotCarriesRunInfo()
  {
    std::ofstream(this->HelperLog()) << "snapshot session";
    this->RunHelperExpectingCleanExit("snapshot");

    mitk::CrashDumpFacility::Initialize(this->MakeConfig(false));
    const auto dumps = mitk::CrashDumpFacility::ListAllDumps();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), dumps.size());
    CPPUNIT_ASSERT(mitk::DumpKind::OnDemand == dumps.front().Kind);
    this->CheckHelperRunInfo(dumps.front());
    CPPUNIT_ASSERT(!AnyReportResidueExists(m_DatabaseDirectory));
    CPPUNIT_ASSERT_MESSAGE("a snapshot must keep the log as it was at capture", !dumps.front().SessionLog.empty());
    CPPUNIT_ASSERT_EQUAL(std::string("snapshot session"), ReadFile(dumps.front().SessionLog));
  }
#endif
};

MITK_TEST_SUITE_REGISTRATION(mitkCrashDumpCapture)
