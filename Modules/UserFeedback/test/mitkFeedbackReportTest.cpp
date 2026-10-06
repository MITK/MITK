/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFeedbackBundle.h>
#include <mitkFeedbackReport.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkFeedbackReportTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkFeedbackReportTestSuite);
  MITK_TEST(ReportTextHasDescriptionAndIncludedItemsOnly);
  MITK_TEST(ReportTextListsIncludedLogsWithEditedNote);
  MITK_TEST(ReportTextListsAttachmentsByDescriptionNeverByPath);
  MITK_TEST(ReportTextStatesTheConfirmationOnlyWhenGiven);
  MITK_TEST(SharingConfirmationIsNeededForEveryPhiCapableItem);
  MITK_TEST(PublicTrackerGate);
  CPPUNIT_TEST_SUITE_END();

  static mitk::FeedbackReport TextOnlyReport()
  {
    mitk::FeedbackReport report;
    report.description = "The viewer freezes when scrolling.";
    report.environment = { { "MITK version", "2026.06", true }, { "Operating system", "Test OS", false } };
    return report;
  }

  static mitk::FeedbackLog Log(bool included, bool edited = false)
  {
    return { "logs/current-session.log", "This session", "a log line\n", included, edited };
  }

  static mitk::FeedbackAttachment Attachment(bool included)
  {
    return { "C:/Users/someone/dumps/crash.dmp", "dumps/crash.dmp", "Crash, 2026-10-06 12:00, 1.2 MB, MITK Test 1.0",
      included };
  }

  static bool Contains(const std::string& text, const std::string& part)
  {
    return text.find(part) != std::string::npos;
  }

public:
  void ReportTextHasDescriptionAndIncludedItemsOnly()
  {
    const auto text = mitk::BuildReportText(TextOnlyReport());

    CPPUNIT_ASSERT(Contains(text, "The viewer freezes when scrolling."));
    CPPUNIT_ASSERT(Contains(text, "Bug report"));
    CPPUNIT_ASSERT(Contains(text, "MITK version: 2026.06"));
    CPPUNIT_ASSERT(!Contains(text, "Test OS"));
    CPPUNIT_ASSERT(!Contains(text, "## Logs"));
    CPPUNIT_ASSERT(!Contains(text, "## Screenshot"));
    CPPUNIT_ASSERT_EQUAL(text, mitk::BuildReportText(TextOnlyReport()));
  }

  void ReportTextListsIncludedLogsWithEditedNote()
  {
    auto report = TextOnlyReport();
    report.logs = { Log(false) };
    CPPUNIT_ASSERT(!Contains(mitk::BuildReportText(report), "This session"));

    report.logs = { Log(true) };
    auto text = mitk::BuildReportText(report);
    CPPUNIT_ASSERT(Contains(text, "This session"));
    CPPUNIT_ASSERT(!Contains(text, "edited"));
    CPPUNIT_ASSERT_MESSAGE("the log text itself goes to the bundle, not the report", !Contains(text, "a log line"));

    report.logs = { Log(true, true) };
    CPPUNIT_ASSERT(Contains(mitk::BuildReportText(report), "edited"));
  }

  void ReportTextListsAttachmentsByDescriptionNeverByPath()
  {
    auto report = TextOnlyReport();
    report.attachments = { Attachment(true) };

    const auto text = mitk::BuildReportText(report);
    CPPUNIT_ASSERT(Contains(text, "Crash, 2026-10-06 12:00, 1.2 MB, MITK Test 1.0"));
    CPPUNIT_ASSERT(Contains(text, "dumps/crash.dmp"));
    CPPUNIT_ASSERT(!Contains(text, "someone"));

    report.attachments = { Attachment(false) };
    CPPUNIT_ASSERT(!Contains(mitk::BuildReportText(report), "crash.dmp"));
  }

  void ReportTextStatesTheConfirmationOnlyWhenGiven()
  {
    auto report = TextOnlyReport();
    report.logs = { Log(true) };
    CPPUNIT_ASSERT(!Contains(mitk::BuildReportText(report), mitk::GetSharingConfirmationStatement()));

    report.sharingConfirmed = true;
    const auto text = mitk::BuildReportText(report);
    CPPUNIT_ASSERT(Contains(text, mitk::GetSharingConfirmationStatement()));
    CPPUNIT_ASSERT(!Contains(text, mitk::GetPublicSharingConfirmationStatement()));

    report.publicSharingConfirmed = true;
    CPPUNIT_ASSERT(Contains(mitk::BuildReportText(report), mitk::GetPublicSharingConfirmationStatement()));
  }

  void SharingConfirmationIsNeededForEveryPhiCapableItem()
  {
    auto report = TextOnlyReport();
    CPPUNIT_ASSERT(!report.NeedsSharingConfirmation());

    report.logs = { Log(false) };
    report.attachments = { Attachment(false) };
    CPPUNIT_ASSERT(!report.NeedsSharingConfirmation());

    report.logs = { Log(true) };
    CPPUNIT_ASSERT(report.NeedsSharingConfirmation());

    report.logs = { Log(false) };
    report.attachments = { Attachment(true) };
    CPPUNIT_ASSERT(report.NeedsSharingConfirmation());

    report.attachments = { Attachment(false) };
    report.includeScreenshot = true;
    CPPUNIT_ASSERT(report.NeedsSharingConfirmation());
  }

  void PublicTrackerGate()
  {
    auto report = TextOnlyReport();
    CPPUNIT_ASSERT(report.CanUsePublicTracker());

    report.logs = { Log(true) };
    CPPUNIT_ASSERT(!report.CanUsePublicTracker());
    report.publicSharingConfirmed = true;
    CPPUNIT_ASSERT(report.CanUsePublicTracker());

    report.logs.clear();
    report.publicSharingConfirmed = false;
    report.includeScreenshot = true;
    CPPUNIT_ASSERT(!report.CanUsePublicTracker());
    report.publicSharingConfirmed = true;
    CPPUNIT_ASSERT(report.CanUsePublicTracker());

    report.includeScreenshot = false;
    report.attachments = { Attachment(true) };
    report.sharingConfirmed = true;
    CPPUNIT_ASSERT_MESSAGE("no confirmation makes an attachment publishable", !report.CanUsePublicTracker());

    report.attachments = { Attachment(false) };
    CPPUNIT_ASSERT(report.CanUsePublicTracker());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkFeedbackReport)
