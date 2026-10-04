/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCrashDumpDatabase.h>

#include <mitkIOUtil.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

class mitkCrashDumpDatabaseTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCrashDumpDatabaseTestSuite);
  MITK_TEST(ScanOrdersNewestFirst);
  MITK_TEST(ScanIgnoresNonDumpFiles);
  MITK_TEST(ScanFindsDumpsInSubdirectories);
  MITK_TEST(ScanMissingDirectoryYieldsEmpty);
  MITK_TEST(PruneKeepsNewestDumps);
  MITK_TEST(PruneIsNoopBelowLimit);
  MITK_TEST(UnacknowledgedIsEverythingWithoutWatermark);
  MITK_TEST(WatermarkHidesOlderDumps);
  MITK_TEST(UnreadableWatermarkCountsAsAbsent);
  MITK_TEST(RemoveCrashReportResidueRemovesMetaAndAttachments);
  MITK_TEST(RemoveCrashReportResidueNeedsAnAbsoluteDatabase);
  MITK_TEST(RemoveCrashReportResidueIgnoresDotNamedReports);
  MITK_TEST(ScanClassifiesDumpsByArea);
  MITK_TEST(PruneRemovesSidecars);
  MITK_TEST(SettingsMissingFileYieldsDefaults);
  MITK_TEST(SettingsRoundTrip);
  MITK_TEST(SettingsMalformedFileYieldsDefaults);
  MITK_TEST(SettingsWrongTypeKeepsDefaultForThatKey);
  MITK_TEST(SettingsAreClampedOnRead);
  MITK_TEST(SettingsAreClampedOnWrite);
  MITK_TEST(SettingsWithoutDatabaseCannotBeWritten);
  MITK_TEST(RunInfoRoundTripsNonAsciiPaths);
  MITK_TEST(MalformedRunInfoIsAbsent);
  MITK_TEST(AdoptRunInfoAttachmentCopiesItOnce);
  MITK_TEST(KeepSessionLogCopiesItOnce);
  MITK_TEST(KeepSessionLogRefusesALaterSessionsLog);
  CPPUNIT_TEST_SUITE_END();

  std::filesystem::path m_DatabaseDirectory;

public:
  void setUp() override
  {
    m_DatabaseDirectory = mitk::IOUtil::CreateTemporaryDirectory("mitkCrashDumpDatabaseTest_XXXXXX");
  }

  void tearDown() override
  {
    std::error_code error;
    std::filesystem::remove_all(m_DatabaseDirectory, error);
  }

  /** Creates a dump file whose age is controlled explicitly, so ordering
   *  tests do not depend on filesystem timestamp granularity. */
  std::filesystem::path CreateDump(const std::string& relativePath, int ageInSeconds)
  {
    const auto path = m_DatabaseDirectory / relativePath;

    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path) << "minidump placeholder " << relativePath;

    std::filesystem::last_write_time(path,
      std::filesystem::file_time_type::clock::now() - std::chrono::seconds(ageInSeconds));

    return path;
  }

  /** The bookkeeping Crashpad keeps for one report, next to its dump. */
  std::filesystem::path CreateReportResidue(const std::filesystem::path& dumpPath)
  {
    const auto reportId = dumpPath.stem();

    std::ofstream(std::filesystem::path(dumpPath).replace_extension(".meta")) << "report metadata";

    const auto attachments = m_DatabaseDirectory / "attachments" / reportId;
    std::filesystem::create_directories(attachments);
    std::ofstream(attachments / "note.txt") << "attachment";

    return attachments;
  }

  void ScanOrdersNewestFirst()
  {
    this->CreateDump("middle.dmp", 20);
    this->CreateDump("newest.dmp", 10);
    this->CreateDump("oldest.dmp", 30);

    const auto dumps = mitk::ScanCrashDumps(m_DatabaseDirectory);

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), dumps.size());
    CPPUNIT_ASSERT_EQUAL(std::string("newest.dmp"), dumps[0].Path.filename().string());
    CPPUNIT_ASSERT_EQUAL(std::string("middle.dmp"), dumps[1].Path.filename().string());
    CPPUNIT_ASSERT_EQUAL(std::string("oldest.dmp"), dumps[2].Path.filename().string());
    CPPUNIT_ASSERT(dumps[0].SizeInBytes > 0);
  }

  void ScanIgnoresNonDumpFiles()
  {
    this->CreateDump("crash.dmp", 10);
    std::ofstream(m_DatabaseDirectory / "settings.dat") << "crashpad bookkeeping";

    const auto dumps = mitk::ScanCrashDumps(m_DatabaseDirectory);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), dumps.size());
    CPPUNIT_ASSERT_EQUAL(std::string("crash.dmp"), dumps[0].Path.filename().string());
  }

  void ScanFindsDumpsInSubdirectories()
  {
    this->CreateDump("reports/nested.dmp", 10);

    const auto dumps = mitk::ScanCrashDumps(m_DatabaseDirectory);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), dumps.size());
    CPPUNIT_ASSERT_EQUAL(std::string("nested.dmp"), dumps[0].Path.filename().string());
  }

  void ScanMissingDirectoryYieldsEmpty()
  {
    const auto dumps = mitk::ScanCrashDumps(m_DatabaseDirectory / "does-not-exist");

    CPPUNIT_ASSERT(dumps.empty());
  }

  void PruneKeepsNewestDumps()
  {
    for (int i = 0; i < 5; ++i)
      this->CreateDump("dump" + std::to_string(i) + ".dmp", 10 * (i + 1));

    const auto deleted = mitk::PruneCrashDumps(m_DatabaseDirectory, 3);

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), deleted);

    const auto dumps = mitk::ScanCrashDumps(m_DatabaseDirectory);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), dumps.size());
    CPPUNIT_ASSERT_EQUAL(std::string("dump0.dmp"), dumps[0].Path.filename().string());
    CPPUNIT_ASSERT_EQUAL(std::string("dump1.dmp"), dumps[1].Path.filename().string());
    CPPUNIT_ASSERT_EQUAL(std::string("dump2.dmp"), dumps[2].Path.filename().string());
  }

  void PruneIsNoopBelowLimit()
  {
    this->CreateDump("only.dmp", 10);

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), mitk::PruneCrashDumps(m_DatabaseDirectory, 3));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::ScanCrashDumps(m_DatabaseDirectory).size());
  }

  void UnacknowledgedIsEverythingWithoutWatermark()
  {
    this->CreateDump("a.dmp", 10);
    this->CreateDump("b.dmp", 20);

    CPPUNIT_ASSERT(!mitk::ReadLastAcknowledgedTime(m_DatabaseDirectory).has_value());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), mitk::ScanUnacknowledgedCrashDumps(m_DatabaseDirectory).size());
  }

  void WatermarkHidesOlderDumps()
  {
    this->CreateDump("new.dmp", 10);
    const auto middle = this->CreateDump("middle.dmp", 20);
    this->CreateDump("old.dmp", 30);

    const auto middleTime = std::filesystem::last_write_time(middle);
    CPPUNIT_ASSERT(mitk::WriteLastAcknowledgedTime(m_DatabaseDirectory, middleTime));

    const auto roundTripped = mitk::ReadLastAcknowledgedTime(m_DatabaseDirectory);
    CPPUNIT_ASSERT(roundTripped.has_value());
    CPPUNIT_ASSERT(*roundTripped == middleTime);

    const auto unacknowledged = mitk::ScanUnacknowledgedCrashDumps(m_DatabaseDirectory);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), unacknowledged.size());
    CPPUNIT_ASSERT_EQUAL(std::string("new.dmp"), unacknowledged[0].Path.filename().string());

    const auto newest = mitk::ScanCrashDumps(m_DatabaseDirectory).front();
    CPPUNIT_ASSERT(mitk::WriteLastAcknowledgedTime(m_DatabaseDirectory, newest.LastWriteTime));
    CPPUNIT_ASSERT(mitk::ScanUnacknowledgedCrashDumps(m_DatabaseDirectory).empty());
  }

  void UnreadableWatermarkCountsAsAbsent()
  {
    this->CreateDump("crash.dmp", 10);
    std::ofstream(mitk::GetAcknowledgedMarkerFilePath(m_DatabaseDirectory)) << "not a timestamp";

    CPPUNIT_ASSERT(!mitk::ReadLastAcknowledgedTime(m_DatabaseDirectory).has_value());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::ScanUnacknowledgedCrashDumps(m_DatabaseDirectory).size());
  }

  void RemoveCrashReportResidueRemovesMetaAndAttachments()
  {
    const auto dump = this->CreateDump("pending/report.dmp", 10);
    const auto attachments = this->CreateReportResidue(dump);
    const auto metadata = std::filesystem::path(dump).replace_extension(".meta");

    mitk::RemoveCrashReportResidue(m_DatabaseDirectory, dump);

    CPPUNIT_ASSERT(!std::filesystem::exists(metadata));
    CPPUNIT_ASSERT(!std::filesystem::exists(attachments));
    CPPUNIT_ASSERT_MESSAGE("only the bookkeeping goes; the dump is the caller's to place",
      std::filesystem::exists(dump));
  }

  /** A caller without facility state passes an empty database directory, and
   *  "attachments/<uuid>" resolved against the working directory would be a
   *  recursive delete somewhere unintended. */
  void RemoveCrashReportResidueNeedsAnAbsoluteDatabase()
  {
    const auto dump = this->CreateDump("pending/report.dmp", 10);
    const auto attachments = this->CreateReportResidue(dump);
    const auto metadata = std::filesystem::path(dump).replace_extension(".meta");

    mitk::RemoveCrashReportResidue({}, dump);
    mitk::RemoveCrashReportResidue("relative/database", dump);

    CPPUNIT_ASSERT(std::filesystem::exists(metadata));
    CPPUNIT_ASSERT(std::filesystem::exists(attachments));
  }

  /** "..dmp" and "...dmp" are dumps as far as the scan is concerned, and their
   *  stems are "." and "..", which would send the recursive delete to the
   *  attachments root and to the database itself. */
  void RemoveCrashReportResidueIgnoresDotNamedReports()
  {
    const auto dump = this->CreateDump("pending/report.dmp", 10);
    const auto attachments = this->CreateReportResidue(dump);

    mitk::RemoveCrashReportResidue(m_DatabaseDirectory, m_DatabaseDirectory / "pending" / "..dmp");
    mitk::RemoveCrashReportResidue(m_DatabaseDirectory, m_DatabaseDirectory / "pending" / "...dmp");

    CPPUNIT_ASSERT_MESSAGE("a dot-named report must not reach the recursive delete",
      std::filesystem::exists(attachments));
    CPPUNIT_ASSERT(std::filesystem::exists(m_DatabaseDirectory / "attachments"));
    CPPUNIT_ASSERT(std::filesystem::exists(dump));
  }

  void WriteSettingsFile(const std::string& content)
  {
    std::ofstream(mitk::GetSettingsFilePath(m_DatabaseDirectory)) << content;
  }

  void ScanClassifiesDumpsByArea()
  {
    this->CreateDump("reports/crash.dmp", 10);
    this->CreateDump("mitk-snapshots/ondemand.dmp", 20);
    this->CreateDump("mitk-pending-freeze/freeze.dmp", 30);

    const auto dumps = mitk::ScanCrashDumps(m_DatabaseDirectory);

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), dumps.size());
    CPPUNIT_ASSERT(mitk::DumpKind::Crash == dumps[0].Kind);
    CPPUNIT_ASSERT(mitk::DumpKind::OnDemand == dumps[1].Kind);
    CPPUNIT_ASSERT(mitk::DumpKind::UnresponsiveTerminated == dumps[2].Kind);

    // A scan rooted in an area must classify the same way.
    const auto snapshots = mitk::ScanCrashDumps(m_DatabaseDirectory / "mitk-snapshots");
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), snapshots.size());
    CPPUNIT_ASSERT(mitk::DumpKind::OnDemand == snapshots[0].Kind);
  }

  void PruneRemovesSidecars()
  {
    const auto kept = this->CreateDump("kept.dmp", 10);
    const auto pruned = this->CreateDump("pruned.dmp", 20);
    std::ofstream(mitk::GetRunInfoSidecarPath(kept)) << "{}";
    std::ofstream(mitk::GetRunInfoSidecarPath(pruned)) << "{}";
    std::ofstream(mitk::GetSessionLogCopyPath(kept)) << "log";
    std::ofstream(mitk::GetSessionLogCopyPath(pruned)) << "log";

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), mitk::PruneCrashDumps(m_DatabaseDirectory, 1));

    CPPUNIT_ASSERT(std::filesystem::exists(mitk::GetRunInfoSidecarPath(kept)));
    CPPUNIT_ASSERT(!std::filesystem::exists(mitk::GetRunInfoSidecarPath(pruned)));
    CPPUNIT_ASSERT(std::filesystem::exists(mitk::GetSessionLogCopyPath(kept)));
    CPPUNIT_ASSERT(!std::filesystem::exists(mitk::GetSessionLogCopyPath(pruned)));
  }

  static std::string ReadFile(const std::filesystem::path& file)
  {
    std::ifstream stream(file);
    return std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
  }

  void KeepSessionLogCopiesItOnce()
  {
    const auto dump = this->CreateDump("reports/report.dmp", 10);
    const auto log = m_DatabaseDirectory / "mitk-0.log";
    std::ofstream(log) << "crashed session";
    std::filesystem::last_write_time(log, std::filesystem::last_write_time(dump) - std::chrono::seconds(5));

    CPPUNIT_ASSERT(mitk::KeepSessionLog(dump, log, std::filesystem::last_write_time(dump)));
    CPPUNIT_ASSERT_EQUAL(std::string("crashed session"), ReadFile(mitk::GetSessionLogCopyPath(dump)));

    mitk::CrashDumpInfo info;
    info.Path = dump;
    mitk::LoadRunInfo(info);
    CPPUNIT_ASSERT(mitk::GetSessionLogCopyPath(dump) == info.SessionLog);

    // A kept copy is final: what the file name holds later is another session.
    std::ofstream(log) << "next session";
    CPPUNIT_ASSERT(mitk::KeepSessionLog(dump, log, std::nullopt));
    CPPUNIT_ASSERT_EQUAL(std::string("crashed session"), ReadFile(mitk::GetSessionLogCopyPath(dump)));
  }

  /** Log rotation reuses the name: a log written after the dump belongs to a
   *  later session of the same install and must not be kept as the dump's. */
  void KeepSessionLogRefusesALaterSessionsLog()
  {
    const auto dump = this->CreateDump("reports/report.dmp", 60);
    const auto log = m_DatabaseDirectory / "mitk-0.log";
    std::ofstream(log) << "later session";

    CPPUNIT_ASSERT(!mitk::KeepSessionLog(dump, log, std::filesystem::last_write_time(dump)));
    CPPUNIT_ASSERT(!std::filesystem::exists(mitk::GetSessionLogCopyPath(dump)));

    mitk::CrashDumpInfo info;
    info.Path = dump;
    mitk::LoadRunInfo(info);
    CPPUNIT_ASSERT(info.SessionLog.empty());
  }

  void SettingsMissingFileYieldsDefaults()
  {
    const auto settings = mitk::ReadCrashDumpSettings(m_DatabaseDirectory);

    CPPUNIT_ASSERT(settings.Enabled);
    CPPUNIT_ASSERT_EQUAL(10, settings.MaxDumpsPerKind);
    CPPUNIT_ASSERT_EQUAL(0, settings.WatchdogTimeoutSeconds);
  }

  void SettingsRoundTrip()
  {
    mitk::CrashDumpSettings settings;
    settings.Enabled = false;
    settings.MaxDumpsPerKind = 3;
    settings.WatchdogTimeoutSeconds = 45;

    CPPUNIT_ASSERT(mitk::WriteCrashDumpSettings(m_DatabaseDirectory, settings));

    const auto read = mitk::ReadCrashDumpSettings(m_DatabaseDirectory);
    CPPUNIT_ASSERT(!read.Enabled);
    CPPUNIT_ASSERT_EQUAL(3, read.MaxDumpsPerKind);
    CPPUNIT_ASSERT_EQUAL(45, read.WatchdogTimeoutSeconds);
  }

  void SettingsMalformedFileYieldsDefaults()
  {
    this->WriteSettingsFile("{ \"enabled\": false, ");

    const auto settings = mitk::ReadCrashDumpSettings(m_DatabaseDirectory);

    CPPUNIT_ASSERT(settings.Enabled);
    CPPUNIT_ASSERT_EQUAL(10, settings.MaxDumpsPerKind);
    CPPUNIT_ASSERT_EQUAL(0, settings.WatchdogTimeoutSeconds);
  }

  void SettingsWrongTypeKeepsDefaultForThatKey()
  {
    this->WriteSettingsFile(R"({ "enabled": "no", "maxDumpsPerKind": 4, "watchdogTimeoutSeconds": 2.5 })");

    const auto settings = mitk::ReadCrashDumpSettings(m_DatabaseDirectory);

    CPPUNIT_ASSERT(settings.Enabled);
    CPPUNIT_ASSERT_EQUAL(4, settings.MaxDumpsPerKind);
    CPPUNIT_ASSERT_EQUAL(0, settings.WatchdogTimeoutSeconds);
  }

  void SettingsAreClampedOnRead()
  {
    this->WriteSettingsFile(R"({ "maxDumpsPerKind": 0, "watchdogTimeoutSeconds": 5 })");
    auto settings = mitk::ReadCrashDumpSettings(m_DatabaseDirectory);
    CPPUNIT_ASSERT_EQUAL(1, settings.MaxDumpsPerKind);
    CPPUNIT_ASSERT_EQUAL(10, settings.WatchdogTimeoutSeconds);

    this->WriteSettingsFile(R"({ "maxDumpsPerKind": 42, "watchdogTimeoutSeconds": 99999 })");
    settings = mitk::ReadCrashDumpSettings(m_DatabaseDirectory);
    CPPUNIT_ASSERT_EQUAL(10, settings.MaxDumpsPerKind);
    CPPUNIT_ASSERT_EQUAL(3600, settings.WatchdogTimeoutSeconds);

    this->WriteSettingsFile(R"({ "watchdogTimeoutSeconds": -3 })");
    settings = mitk::ReadCrashDumpSettings(m_DatabaseDirectory);
    CPPUNIT_ASSERT_EQUAL(0, settings.WatchdogTimeoutSeconds);
  }

  void SettingsAreClampedOnWrite()
  {
    mitk::CrashDumpSettings settings;
    settings.MaxDumpsPerKind = 11;
    settings.WatchdogTimeoutSeconds = 1;

    CPPUNIT_ASSERT(mitk::WriteCrashDumpSettings(m_DatabaseDirectory, settings));

    std::ifstream file(mitk::GetSettingsFilePath(m_DatabaseDirectory));
    const std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    CPPUNIT_ASSERT(content.find("11") == std::string::npos);

    const auto read = mitk::ReadCrashDumpSettings(m_DatabaseDirectory);
    CPPUNIT_ASSERT_EQUAL(10, read.MaxDumpsPerKind);
    CPPUNIT_ASSERT_EQUAL(10, read.WatchdogTimeoutSeconds);
  }

  void SettingsWithoutDatabaseCannotBeWritten()
  {
    CPPUNIT_ASSERT(!mitk::WriteCrashDumpSettings({}, mitk::CrashDumpSettings()));
  }

  void RunInfoRoundTripsNonAsciiPaths()
  {
    mitk::CrashRunInfo runInfo;
    runInfo.Release = "MITK Workbench 2026.10";
    runInfo.InstallDirectory = std::filesystem::path(u8"C:/Programme/M\u00fcller/MITK");
    runInfo.LogFile = std::filesystem::path(u8"C:/Users/J\u00fcrgen/mitk-0.log");

    const auto file = m_DatabaseDirectory / "run-info.json";
    CPPUNIT_ASSERT(mitk::WriteRunInfo(file, runInfo));
    CPPUNIT_ASSERT_MESSAGE("the atomic replace must not leave its temporary behind",
      !std::filesystem::exists(m_DatabaseDirectory / "run-info.json.tmp"));

    const auto read = mitk::ReadRunInfo(file);
    CPPUNIT_ASSERT(read.has_value());
    CPPUNIT_ASSERT_EQUAL(runInfo.Release, read->Release);
    CPPUNIT_ASSERT(runInfo.InstallDirectory == read->InstallDirectory);
    CPPUNIT_ASSERT(runInfo.LogFile == read->LogFile);

    // Rewriting replaces the content.
    runInfo.LogFile.clear();
    CPPUNIT_ASSERT(mitk::WriteRunInfo(file, runInfo));
    CPPUNIT_ASSERT(mitk::ReadRunInfo(file)->LogFile.empty());
  }

  void MalformedRunInfoIsAbsent()
  {
    const auto file = m_DatabaseDirectory / "run-info.json";
    std::ofstream(file) << "not json";

    CPPUNIT_ASSERT(!mitk::ReadRunInfo(file).has_value());
    CPPUNIT_ASSERT(!mitk::ReadRunInfo(m_DatabaseDirectory / "missing.json").has_value());
  }

  void AdoptRunInfoAttachmentCopiesItOnce()
  {
    const auto dump = this->CreateDump("reports/report.dmp", 10);
    const auto attachment = m_DatabaseDirectory / "attachments" / "report" / mitk::RunInfoAttachmentFileName;

    CPPUNIT_ASSERT_MESSAGE("no attachment, no sidecar",
      !mitk::AdoptRunInfoAttachment(m_DatabaseDirectory, "report", dump));

    std::filesystem::create_directories(attachment.parent_path());
    mitk::CrashRunInfo runInfo;
    runInfo.Release = "first";
    CPPUNIT_ASSERT(mitk::WriteRunInfo(attachment, runInfo));

    CPPUNIT_ASSERT(mitk::AdoptRunInfoAttachment(m_DatabaseDirectory, "report", dump));

    mitk::CrashDumpInfo info;
    info.Path = dump;
    mitk::LoadRunInfo(info);
    CPPUNIT_ASSERT(info.RunInfo.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("first"), info.RunInfo->Release);

    // An existing sidecar is never overwritten, so the run info stays
    // attached to the dump once the report residue is gone.
    runInfo.Release = "second";
    CPPUNIT_ASSERT(mitk::WriteRunInfo(attachment, runInfo));
    CPPUNIT_ASSERT(mitk::AdoptRunInfoAttachment(m_DatabaseDirectory, "report", dump));
    mitk::LoadRunInfo(info);
    CPPUNIT_ASSERT_EQUAL(std::string("first"), info.RunInfo->Release);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkCrashDumpDatabase)
