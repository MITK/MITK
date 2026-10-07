/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFeedbackBundle.h>

#include <mitkIOUtil.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <Poco/StreamCopier.h>
#include <Poco/Zip/ZipArchive.h>
#include <Poco/Zip/ZipStream.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

class mitkFeedbackBundleTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkFeedbackBundleTestSuite);
  MITK_TEST(TextOnlyBundleHasReportAndManifest);
  MITK_TEST(BundleHoldsExactlyTheIncludedItems);
  MITK_TEST(EditedLogStartsWithTheMarker);
  MITK_TEST(ManifestListsSizesAndConfirmations);
  MITK_TEST(SameInputGivesSameEntries);
  MITK_TEST(MissingAttachmentThrowsAndLeavesNoZip);
  MITK_TEST(UnsafeArchiveNamesThrow);
  MITK_TEST(UserImagesAreCopiedAndListed);
  CPPUNIT_TEST_SUITE_END();

  std::filesystem::path m_Directory;
  std::filesystem::path m_Dump;
  std::filesystem::path m_Zip;
  std::string m_DumpContent;

  /** File entries of the zip at \p path, by name. */
  static std::map<std::string, std::string> ReadZip(const std::filesystem::path& path)
  {
    std::ifstream file(path, std::ios::binary);
    const Poco::Zip::ZipArchive archive(file);

    std::map<std::string, std::string> entries;

    for (auto header = archive.headerBegin(); header != archive.headerEnd(); ++header)
    {
      if (!header->second.isFile())
        continue;

      Poco::Zip::ZipInputStream entry(file, header->second);
      std::ostringstream content;
      Poco::StreamCopier::copyStream(entry, content);
      entries[header->first] = content.str();
    }

    return entries;
  }

  mitk::FeedbackReport FullReport() const
  {
    mitk::FeedbackReport report;
    report.description = "Crash when saving.";
    report.environment = { { "MITK version", "2026.06", true } };
    report.logs = {
      { "logs/current-session.log", "This session", "current log\n", true, false },
      { "logs/crash.log", "Session log of the crash", "crash log\n", false, false }
    };
    report.attachments = { { m_Dump, "dumps/crash.dmp", "Crash, 1 kB", true } };
    report.includeScreenshot = true;
    report.screenshotPng = { 0x89, 'P', 'N', 'G' };
    report.sharingConfirmed = true;
    return report;
  }

public:
  void setUp() override
  {
    m_Directory = mitk::IOUtil::CreateTemporaryDirectory("mitkFeedbackBundleTest_XXXXXX");
    m_Dump = m_Directory / "crash.dmp";
    m_Zip = m_Directory / "bundle.zip";

    m_DumpContent.clear();
    for (int i = 0; i < 5000; ++i)
      m_DumpContent += static_cast<char>(i * 7);

    std::ofstream(m_Dump, std::ios::binary) << m_DumpContent;
  }

  void tearDown() override
  {
    std::error_code error;
    std::filesystem::remove_all(m_Directory, error);
  }

  void TextOnlyBundleHasReportAndManifest()
  {
    mitk::FeedbackReport report;
    report.description = "An idea.";
    report.category = mitk::FeedbackCategory::Idea;

    mitk::WriteFeedbackBundle(report, m_Zip);
    const auto entries = ReadZip(m_Zip);

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), entries.size());
    CPPUNIT_ASSERT_EQUAL(mitk::BuildReportText(report), entries.at("report.md"));
    CPPUNIT_ASSERT(entries.count("manifest.txt") == 1);
  }

  void BundleHoldsExactlyTheIncludedItems()
  {
    const auto report = this->FullReport();

    mitk::WriteFeedbackBundle(report, m_Zip);
    const auto entries = ReadZip(m_Zip);

    CPPUNIT_ASSERT_EQUAL(std::size_t(5), entries.size());
    CPPUNIT_ASSERT_EQUAL(mitk::BuildReportText(report), entries.at("report.md"));
    CPPUNIT_ASSERT_EQUAL(std::string("current log\n"), entries.at("logs/current-session.log"));
    CPPUNIT_ASSERT(entries.count("logs/crash.log") == 0);
    CPPUNIT_ASSERT_EQUAL(std::string("\x89PNG"), entries.at("screenshot.png"));
    CPPUNIT_ASSERT_MESSAGE("attachments are copied byte-identical", m_DumpContent == entries.at("dumps/crash.dmp"));
    CPPUNIT_ASSERT(entries.count("manifest.txt") == 1);

    auto without = report;
    without.includeScreenshot = false;
    without.attachments.front().included = false;
    without.logs.front().included = false;

    mitk::WriteFeedbackBundle(without, m_Zip);
    const auto reduced = ReadZip(m_Zip);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), reduced.size());
    CPPUNIT_ASSERT(reduced.count("report.md") == 1);
    CPPUNIT_ASSERT(reduced.count("manifest.txt") == 1);
  }

  void EditedLogStartsWithTheMarker()
  {
    auto report = this->FullReport();
    report.logs.front().edited = true;

    mitk::WriteFeedbackBundle(report, m_Zip);
    const auto log = ReadZip(m_Zip).at("logs/current-session.log");

    const auto firstLineEnd = log.find('\n');
    CPPUNIT_ASSERT(log.substr(0, firstLineEnd).find("edited") != std::string::npos);
    CPPUNIT_ASSERT_EQUAL(std::string("current log\n"), log.substr(firstLineEnd + 1));
  }

  void ManifestListsSizesAndConfirmations()
  {
    auto report = this->FullReport();
    report.logs.front().edited = true;

    mitk::WriteFeedbackBundle(report, m_Zip);
    const auto entries = ReadZip(m_Zip);
    const auto& manifest = entries.at("manifest.txt");

    const auto line = [&manifest](const std::string& name) {
      const auto begin = manifest.find(name + '\t');
      CPPUNIT_ASSERT_MESSAGE(name, begin != std::string::npos);
      return manifest.substr(begin, manifest.find('\n', begin) - begin);
    };

    CPPUNIT_ASSERT_EQUAL("report.md\t" + std::to_string(entries.at("report.md").size()) + " bytes", line("report.md"));
    CPPUNIT_ASSERT_EQUAL("logs/current-session.log\t" + std::to_string(entries.at("logs/current-session.log").size()) +
      " bytes\tedited", line("logs/current-session.log"));
    CPPUNIT_ASSERT_EQUAL(std::string("screenshot.png\t4 bytes"), line("screenshot.png"));
    CPPUNIT_ASSERT_EQUAL("dumps/crash.dmp\t" + std::to_string(m_DumpContent.size()) + " bytes", line("dumps/crash.dmp"));
    CPPUNIT_ASSERT(manifest.find("Sharing confirmed: yes") != std::string::npos);
    CPPUNIT_ASSERT(manifest.find("Public sharing confirmed: no") != std::string::npos);
  }

  void UserImagesAreCopiedAndListed()
  {
    const auto image = m_Directory / "my picture.png";
    std::ofstream(image, std::ios::binary) << "\x89PNG user image";

    auto report = this->FullReport();
    report.images = { { image, "images/1-my picture.png", "my picture.png", true },
      { m_Directory / "missing-but-excluded.png", "images/2-missing.png", "missing.png", false } };

    mitk::WriteFeedbackBundle(report, m_Zip);
    const auto entries = ReadZip(m_Zip);

    CPPUNIT_ASSERT_EQUAL(std::string("\x89PNG user image"), entries.at("images/1-my picture.png"));
    CPPUNIT_ASSERT(entries.count("images/2-missing.png") == 0);
    CPPUNIT_ASSERT(entries.at("manifest.txt").find("images/1-my picture.png\t") != std::string::npos);

    report.images.back().included = true;
    CPPUNIT_ASSERT_THROW_MESSAGE("an included image must exist", mitk::WriteFeedbackBundle(report, m_Zip),
      mitk::FeedbackBundleException);

    report.images.back().sourcePath = image;
    report.images.back().archiveName = "../escape.png";
    CPPUNIT_ASSERT_THROW_MESSAGE("image names are validated", mitk::WriteFeedbackBundle(report, m_Zip),
      mitk::FeedbackBundleException);
  }

  void SameInputGivesSameEntries()
  {
    const auto report = this->FullReport();

    mitk::WriteFeedbackBundle(report, m_Zip);
    const auto first = ReadZip(m_Zip);
    mitk::WriteFeedbackBundle(report, m_Zip);

    CPPUNIT_ASSERT(first == ReadZip(m_Zip));
  }

  void MissingAttachmentThrowsAndLeavesNoZip()
  {
    auto report = this->FullReport();
    report.attachments.front().sourcePath = m_Directory / "missing.dmp";

    CPPUNIT_ASSERT_THROW(mitk::WriteFeedbackBundle(report, m_Zip), mitk::FeedbackBundleException);
    CPPUNIT_ASSERT(!std::filesystem::exists(m_Zip));
  }

  void UnsafeArchiveNamesThrow()
  {
    for (const std::string name : { "../escape.dmp", "dumps/../../escape.dmp", "/absolute.dmp", "C:/absolute.dmp",
           "dumps\\..\\escape.dmp", "", "report.md", "dumps//crash.dmp" })
    {
      auto report = this->FullReport();
      report.attachments.front().archiveName = name;

      CPPUNIT_ASSERT_THROW_MESSAGE(name, mitk::WriteFeedbackBundle(report, m_Zip), mitk::FeedbackBundleException);
      CPPUNIT_ASSERT_MESSAGE(name, !std::filesystem::exists(m_Zip));
    }

    auto report = this->FullReport();
    report.logs.back().included = true;
    report.logs.back().archiveName = report.logs.front().archiveName;
    CPPUNIT_ASSERT_THROW_MESSAGE("duplicate names", mitk::WriteFeedbackBundle(report, m_Zip),
      mitk::FeedbackBundleException);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkFeedbackBundle)
