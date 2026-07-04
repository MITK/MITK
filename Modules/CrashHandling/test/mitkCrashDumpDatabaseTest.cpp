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
};

MITK_TEST_SUITE_REGISTRATION(mitkCrashDumpDatabase)
