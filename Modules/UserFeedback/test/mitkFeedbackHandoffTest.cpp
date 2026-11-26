/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFeedbackHandoff.h>

#include <mitkException.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkFeedbackHandoffTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkFeedbackHandoffTestSuite);
  MITK_TEST(GitHubIsRefusedUnlessThePublicTrackerMayBeUsed);
  MITK_TEST(GitHubLabelsFollowTheCategory);
  MITK_TEST(GitHubTitleIsTheCappedFirstLine);
  MITK_TEST(GitHubBodyFollowsTheTemplateAndNamesTheBundle);
  MITK_TEST(GitHubBodyIsCapped);
  MITK_TEST(BundleFileNameMustBeBare);
  MITK_TEST(MailGoesToTheTeamWithAShortBody);
  MITK_TEST(MailWithManyLogsStaysUnderTheCapAndKeepsTheDescription);
  MITK_TEST(MailCountsASingleLog);
  MITK_TEST(GitHubBodyEscapesHtmlInTheDescription);
  MITK_TEST(MailBodyKeepsTheDescriptionVerbatim);
  MITK_TEST(UserImagesMayGoToGitHub);
  MITK_TEST(BodiesOpenWithAReminderToAttachTheBundle);
  CPPUNIT_TEST_SUITE_END();

  const std::string m_Bundle = "mitk-report-20261006-120000.zip";

  static mitk::FeedbackReport Report(mitk::FeedbackCategory category = mitk::FeedbackCategory::Bug)
  {
    mitk::FeedbackReport report;
    report.category = category;
    report.description = "\n  Crash when saving a scene  \nSteps: open, save.";
    report.environment = {
      { mitk::EnvironmentField::MitkVersion, "2026.06", true },
      { mitk::EnvironmentField::RevisionDescription, "v2026.06-1-gabc", true },
      { mitk::EnvironmentField::OperatingSystem, "Test OS 1", true },
      { mitk::EnvironmentField::CpuArchitecture, "x86_64", false }
    };
    return report;
  }

  static bool Contains(const std::string& text, const std::string& part)
  {
    return text.find(part) != std::string::npos;
  }

  /** Length after percent-encoding everything but the unreserved URI
   *  characters, which is what goes into the URL. */
  static std::size_t EncodedLength(const std::string& text)
  {
    std::size_t length = 0;

    for (const unsigned char c : text)
    {
      const bool unreserved = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
        c == '-' || c == '.' || c == '_' || c == '~';

      length += unreserved ? 1 : 3;
    }

    return length;
  }

  /** About 6000 bytes of the kind of text that inflates most when encoded. */
  static std::string LongGermanDescription()
  {
    std::string description;

    while (description.size() < 6000)
      description += "Fehler beim \xC3\x96" "ffnen, \xC3\xA4" "hnlich wie zuvor:\n";

    return description;
  }

  /** \p count included logs whose labels differ, as the capture of several
   *  sessions produces them. */
  static std::vector<mitk::FeedbackLog> IncludedLogs(int count)
  {
    std::vector<mitk::FeedbackLog> logs;

    for (int i = 0; i < count; ++i)
    {
      const std::string minute = (i < 10 ? "0" : "") + std::to_string(i);

      logs.push_back({ "logs/captured-" + std::to_string(i) + ".log",
        "Session log (Captured on request, 2026-10-07 10:" + minute + ')', "a log line\n", true, false });
    }

    return logs;
  }

  /** A '<' that starts HTML in Markdown (before "int" and "!--") next to the
   *  same character inside an inline code span and a fenced block, and as
   *  plain text before a space and a digit. */
  static std::string HtmlLikeDescription()
  {
    return "crash in std::vector<int> and `std::map<int, int>` here\n"
           "```\nvector<int> v;\n```\n"
           "<!-- note\n"
           "a < b and x<3";
  }

public:
  void GitHubIsRefusedUnlessThePublicTrackerMayBeUsed()
  {
    auto report = Report();
    report.attachments = { { "C:/dumps/crash.dmp", "dumps/crash.dmp", "Crash", true } };
    report.sharingConfirmed = true;
    report.publicSharingConfirmed = true;
    CPPUNIT_ASSERT_THROW(mitk::BuildGitHubIssueFields(report, m_Bundle), mitk::Exception);

    report.attachments.clear();
    report.includeScreenshot = true;
    report.publicSharingConfirmed = false;
    CPPUNIT_ASSERT_THROW(mitk::BuildGitHubIssueFields(report, m_Bundle), mitk::Exception);

    report.publicSharingConfirmed = true;
    CPPUNIT_ASSERT_NO_THROW(mitk::BuildGitHubIssueFields(report, m_Bundle));
  }

  void GitHubLabelsFollowTheCategory()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("bug"), mitk::BuildGitHubIssueFields(Report(), m_Bundle).labels);
    CPPUNIT_ASSERT_EQUAL(std::string("enhancement"),
      mitk::BuildGitHubIssueFields(Report(mitk::FeedbackCategory::Feature), m_Bundle).labels);
    CPPUNIT_ASSERT_EQUAL(std::string("enhancement"),
      mitk::BuildGitHubIssueFields(Report(mitk::FeedbackCategory::Idea), m_Bundle).labels);
  }

  void GitHubTitleIsTheCappedFirstLine()
  {
    auto report = Report();
    CPPUNIT_ASSERT_EQUAL(std::string("Crash when saving a scene"), mitk::BuildGitHubIssueFields(report, m_Bundle).title);

    // Multi-byte characters straddle the cap and must not be split.
    report.description.clear();
    for (int i = 0; i < 60; ++i)
      report.description += "\xC3\xA4";

    const auto title = mitk::BuildGitHubIssueFields(report, m_Bundle).title;
    CPPUNIT_ASSERT(title.size() <= 80);
    CPPUNIT_ASSERT_EQUAL(std::string("..."), title.substr(title.size() - 3));
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), (title.size() - 3) % 2);

    report.description = "   ";
    CPPUNIT_ASSERT_EQUAL(std::string("Bug report"), mitk::BuildGitHubIssueFields(report, m_Bundle).title);
  }

  void GitHubBodyFollowsTheTemplateAndNamesTheBundle()
  {
    auto report = Report();
    report.logs = { { "logs/current-session.log", "This session", "secret log line\n", true, false } };
    report.publicSharingConfirmed = true;

    const auto bug = mitk::BuildGitHubIssueFields(report, m_Bundle).body;

    for (const auto* section : { "**Describe the bug**", "**MITK version**", "**Operating system**",
           "**To Reproduce**", "**Expected behavior**", "**Screenshots**", "**Additional context**" })
    {
      CPPUNIT_ASSERT_MESSAGE(section, Contains(bug, section));
    }

    CPPUNIT_ASSERT(Contains(bug, "Crash when saving a scene"));
    CPPUNIT_ASSERT(Contains(bug, "2026.06 (v2026.06-1-gabc)"));
    CPPUNIT_ASSERT(Contains(bug, "Test OS 1"));
    CPPUNIT_ASSERT(!Contains(bug, "x86_64"));
    CPPUNIT_ASSERT(Contains(bug, "Bundle file: " + m_Bundle + "\n"));
    CPPUNIT_ASSERT(Contains(bug, "This session"));
    CPPUNIT_ASSERT_MESSAGE("logs never go into the URL", !Contains(bug, "secret log line"));

    const auto feature = mitk::BuildGitHubIssueFields(Report(mitk::FeedbackCategory::Feature), m_Bundle).body;

    for (const auto* section : { "**Is your feature request related to a problem? Please describe.**",
           "**Describe the solution you'd like**", "**Describe alternatives you've considered**",
           "**Additional context**" })
    {
      CPPUNIT_ASSERT_MESSAGE(section, Contains(feature, section));
    }

    CPPUNIT_ASSERT(Contains(feature, "Bundle file: " + m_Bundle));
  }

  void GitHubBodyIsCapped()
  {
    auto report = Report();
    report.description = std::string(20000, 'x');

    auto body = mitk::BuildGitHubIssueFields(report, m_Bundle).body;
    CPPUNIT_ASSERT(EncodedLength(body) <= 6000);
    CPPUNIT_ASSERT(Contains(body, "truncated"));
    CPPUNIT_ASSERT_MESSAGE("the bundle reference survives the cap", Contains(body, "Bundle file: " + m_Bundle));

    // GitHub refuses new-issue URLs beyond about 8 kB, so the cap applies to
    // the encoded body, which can be three times as long as the text.
    report.description = LongGermanDescription();
    body = mitk::BuildGitHubIssueFields(report, m_Bundle).body;
    CPPUNIT_ASSERT_MESSAGE(std::to_string(EncodedLength(body)), EncodedLength(body) <= 6000);
    CPPUNIT_ASSERT(Contains(body, "Bundle file: " + m_Bundle));
  }

  void UserImagesMayGoToGitHub()
  {
    auto report = Report();
    report.images = { { "C:/Users/someone/shot.png", "images/1-shot.png", "shot.png", true },
      { "C:/Users/someone/other.png", "images/2-other.png", "other.png", true } };
    CPPUNIT_ASSERT_THROW(mitk::BuildGitHubIssueFields(report, m_Bundle), mitk::Exception);

    report.publicSharingConfirmed = true;
    const auto body = mitk::BuildGitHubIssueFields(report, m_Bundle).body;
    CPPUNIT_ASSERT(Contains(body, "2 images"));
    CPPUNIT_ASSERT(!Contains(body, "someone"));
    CPPUNIT_ASSERT(!Contains(body, "**Screenshots**\nNone."));
  }

  void BundleFileNameMustBeBare()
  {
    for (const std::string name : { "C:\\Users\\someone\\AppData\\Local\\Temp\\report.zip", "/tmp/report.zip", "" })
    {
      CPPUNIT_ASSERT_THROW_MESSAGE(name, mitk::BuildGitHubIssueFields(Report(), name), mitk::Exception);
      CPPUNIT_ASSERT_THROW_MESSAGE(name, mitk::BuildMailFields(Report(), name), mitk::Exception);
    }
  }

  void MailGoesToTheTeamWithAShortBody()
  {
    auto report = Report();
    report.attachments = { { "C:/Users/someone/crash.dmp", "dumps/crash.dmp", "Crash", true } };

    auto mail = mitk::BuildMailFields(report, m_Bundle);
    CPPUNIT_ASSERT_EQUAL(std::string("mitk@dkfz.de"), mail.recipient);
    CPPUNIT_ASSERT(Contains(mail.subject, "Crash when saving a scene"));
    CPPUNIT_ASSERT(Contains(mail.body, m_Bundle));
    CPPUNIT_ASSERT(Contains(mail.body, "diagnostic data file"));
    CPPUNIT_ASSERT(!Contains(mail.body, "someone"));

    // Some mail clients cut mailto: URLs beyond about 2 kB, so the cap
    // applies to the encoded subject and body together.
    for (const auto& description : { std::string(20000, 'x'), LongGermanDescription() })
    {
      report.description = description;
      mail = mitk::BuildMailFields(report, m_Bundle);

      const auto encoded = EncodedLength(mail.subject) + EncodedLength(mail.body);
      CPPUNIT_ASSERT_MESSAGE(std::to_string(encoded), encoded <= 1900);
      CPPUNIT_ASSERT(Contains(mail.body, m_Bundle));
    }
  }

  void MailWithManyLogsStaysUnderTheCapAndKeepsTheDescription()
  {
    auto report = Report();
    report.logs = IncludedLogs(14);
    report.description = std::string(20000, 'x');
    report.sharingConfirmed = true;

    const auto mail = mitk::BuildMailFields(report, m_Bundle);

    // The text around the description must not grow with the number of logs,
    // or the cap leaves no room for the description.
    const auto encoded = EncodedLength(mail.subject) + EncodedLength(mail.body);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(encoded), encoded <= 1900);
    CPPUNIT_ASSERT_MESSAGE(mail.body, Contains(mail.body, std::string(50, 'x')));
    CPPUNIT_ASSERT_MESSAGE(mail.body, Contains(mail.body, "14 log excerpts"));
    CPPUNIT_ASSERT_MESSAGE(mail.body, !Contains(mail.body, "Captured on request"));
    CPPUNIT_ASSERT(Contains(mail.body, m_Bundle));
  }

  void MailCountsASingleLog()
  {
    auto report = Report();
    report.logs = IncludedLogs(2);
    report.logs.back().included = false;
    report.sharingConfirmed = true;

    const auto body = mitk::BuildMailFields(report, m_Bundle).body;

    CPPUNIT_ASSERT_MESSAGE(body, Contains(body, "a log excerpt"));
    CPPUNIT_ASSERT_MESSAGE("a log that is not included is not counted", !Contains(body, "log excerpts"));

    for (const auto& log : report.logs)
    {
      CPPUNIT_ASSERT_MESSAGE(log.sourceLabel, !Contains(body, log.sourceLabel));
    }
  }

  void GitHubBodyEscapesHtmlInTheDescription()
  {
    auto report = Report();
    report.description = HtmlLikeDescription();

    const auto fields = mitk::BuildGitHubIssueFields(report, m_Bundle);

    CPPUNIT_ASSERT_MESSAGE(fields.body, Contains(fields.body, "std::vector\\<int>"));
    CPPUNIT_ASSERT_MESSAGE(fields.body, Contains(fields.body, "`std::map<int, int>`"));
    CPPUNIT_ASSERT_MESSAGE(fields.body, Contains(fields.body, "\nvector<int> v;\n"));
    CPPUNIT_ASSERT_MESSAGE(fields.body, Contains(fields.body, "\\<!-- note"));
    CPPUNIT_ASSERT_MESSAGE(fields.body, Contains(fields.body, "a < b and x<3"));

    // GitHub shows the title as plain text, so a backslash would stay visible.
    CPPUNIT_ASSERT_MESSAGE(fields.title, Contains(fields.title, "std::vector<int>"));
    CPPUNIT_ASSERT_MESSAGE(fields.title, !Contains(fields.title, "\\<"));
  }

  void MailBodyKeepsTheDescriptionVerbatim()
  {
    auto report = Report();
    report.description = HtmlLikeDescription();

    const auto mail = mitk::BuildMailFields(report, m_Bundle);

    // A mail is plain text, so a backslash would stay visible.
    CPPUNIT_ASSERT_MESSAGE(mail.body, Contains(mail.body, "std::vector<int>"));
    CPPUNIT_ASSERT_MESSAGE(mail.body, Contains(mail.body, "<!-- note"));
    CPPUNIT_ASSERT_MESSAGE(mail.body, !Contains(mail.body, "\\<"));
    CPPUNIT_ASSERT_MESSAGE(mail.subject, Contains(mail.subject, "std::vector<int>"));
    CPPUNIT_ASSERT_MESSAGE(mail.subject, !Contains(mail.subject, "\\<"));
  }

  void BodiesOpenWithAReminderToAttachTheBundle()
  {
    // Neither hand-off can attach a file, so the body is the last place
    // that reminds the user to do so before sending.
    auto report = Report();
    const auto issue = mitk::BuildGitHubIssueFields(report, m_Bundle).body;
    CPPUNIT_ASSERT_MESSAGE(issue, issue.rfind("**Before submitting:** drag " + m_Bundle + " ", 0) == 0);

    report.description = std::string(20000, 'x');
    const auto mail = mitk::BuildMailFields(report, m_Bundle).body;
    CPPUNIT_ASSERT_MESSAGE(mail, mail.rfind("[Before sending: attach " + m_Bundle + " ", 0) == 0);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkFeedbackHandoff)
