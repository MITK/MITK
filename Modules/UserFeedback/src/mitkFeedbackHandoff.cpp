/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFeedbackHandoff.h>

#include <mitkExceptionMacro.h>

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
  constexpr std::size_t MaxTitleLength = 80;

  // GitHub rejects new-issue URLs beyond about 8 kB, and URL encoding
  // inflates the body further.
  constexpr std::size_t MaxIssueBodyLength = 6000;

  // mailto: URLs of more than about 2 kB are cut off or refused by some mail
  // clients.
  constexpr std::size_t MaxMailBodyLength = 2000;

  const std::string TruncationNote = " [...] (truncated; the full text is in report.md in the bundle)";

  void ValidateBundleFileName(const std::string& bundleFileName)
  {
    if (bundleFileName.empty() || bundleFileName.find_first_of("/\\:") != std::string::npos)
      mitkThrow() << "Expected a bare bundle file name, got \"" << bundleFileName << "\"";
  }

  /** Cuts \p text to at most \p maxLength bytes without splitting a UTF-8
   *  sequence. */
  std::string CutUtf8(const std::string& text, std::size_t maxLength)
  {
    if (text.size() <= maxLength)
      return text;

    auto length = maxLength;

    while (length > 0 && (static_cast<unsigned char>(text[length]) & 0xC0) == 0x80)
      --length;

    return text.substr(0, length);
  }

  std::string Shorten(const std::string& text, std::size_t maxLength)
  {
    if (text.size() <= maxLength)
      return text;

    const auto room = maxLength > TruncationNote.size() ? maxLength - TruncationNote.size() : 0;
    return CutUtf8(text, room) + TruncationNote;
  }

  std::string Trim(const std::string& text)
  {
    const auto first = text.find_first_not_of(" \t\r\n");

    if (first == std::string::npos)
      return {};

    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
  }

  std::string BuildTitle(const mitk::FeedbackReport& report)
  {
    std::istringstream lines(report.description);
    std::string line;

    while (std::getline(lines, line))
    {
      line = Trim(line);

      if (line.empty())
        continue;

      if (line.size() <= MaxTitleLength)
        return line;

      return CutUtf8(line, MaxTitleLength - 3) + "...";
    }

    return mitk::ToString(report.category);
  }

  /** The included environment value labeled \p label, or empty. */
  std::string IncludedValue(const mitk::FeedbackReport& report, const std::string& label)
  {
    for (const auto& item : report.environment)
    {
      if (item.included && item.label == label)
        return item.value;
    }

    return {};
  }

  std::string OrNotIncluded(const std::string& value)
  {
    return value.empty() ? std::string("Not included.") : value;
  }

  std::string BundleContents(const mitk::FeedbackReport& report)
  {
    std::ostringstream contents;
    contents << "The bundle contains the full report (report.md)";

    for (const auto& log : report.logs)
    {
      if (log.included)
        contents << ", the log excerpt \"" << log.sourceLabel << '"';
    }

    if (report.includeScreenshot)
      contents << ", a screenshot";

    const auto attachments = std::count_if(report.attachments.begin(), report.attachments.end(),
      [](const auto& attachment) { return attachment.included; });

    if (attachments == 1)
      contents << ", a diagnostic data file";
    else if (attachments > 1)
      contents << ", " << attachments << " diagnostic data files";

    contents << '.';
    return contents.str();
  }

  /** Everything below the description, which is the part of the body that
   *  may be shortened. */
  std::string AdditionalContext(const mitk::FeedbackReport& report, const std::string& bundleFileName)
  {
    std::ostringstream context;
    context << "**Additional context**\n"
            << "Bundle file: " << bundleFileName << " - please attach it to this issue\n"
            << BundleContents(report) << '\n';

    bool hasEnvironment = false;

    for (const auto& item : report.environment)
    {
      if (!item.included)
        continue;

      if (!hasEnvironment)
        context << "\nEnvironment:\n";

      hasEnvironment = true;
      context << "- " << item.label << ": " << item.value << '\n';
    }

    return context.str();
  }

  std::string BuildBugBody(const mitk::FeedbackReport& report, const std::string& description,
    const std::string& additionalContext)
  {
    auto version = IncludedValue(report, "MITK version");
    const auto revision = IncludedValue(report, "Revision description");

    if (!version.empty() && !revision.empty())
      version += " (" + revision + ')';

    std::ostringstream body;
    body << "**Describe the bug**\n" << description << "\n\n"
         << "**MITK version**\n" << OrNotIncluded(version) << "\n\n"
         << "**Operating system**\n" << OrNotIncluded(IncludedValue(report, "Operating system")) << "\n\n"
         << "**To Reproduce**\nSee the description above.\n\n"
         << "**Expected behavior**\nSee the description above.\n\n"
         << "**Screenshots**\n"
         << (report.includeScreenshot ? "A screenshot is in the bundle." : std::string("None.")) << "\n\n"
         << additionalContext;

    return body.str();
  }

  std::string BuildFeatureBody(const std::string& description, const std::string& additionalContext)
  {
    std::ostringstream body;
    body << "**Is your feature request related to a problem? Please describe.**\n" << description << "\n\n"
         << "**Describe the solution you'd like**\nSee the description above.\n\n"
         << "**Describe alternatives you've considered**\nNone stated.\n\n"
         << additionalContext;

    return body.str();
  }

  std::string BuildIssueBody(const mitk::FeedbackReport& report, const std::string& description,
    const std::string& additionalContext)
  {
    return report.category == mitk::FeedbackCategory::Bug
      ? BuildBugBody(report, description, additionalContext)
      : BuildFeatureBody(description, additionalContext);
  }
}

std::string mitk::GetFeedbackMailRecipient()
{
  return "mitk@dkfz.de";
}

mitk::GitHubIssueFields mitk::BuildGitHubIssueFields(const FeedbackReport& report, const std::string& bundleFileName)
{
  if (!report.CanUsePublicTracker())
    mitkThrow() << "This report must not be sent to a public issue tracker";

  ValidateBundleFileName(bundleFileName);

  GitHubIssueFields fields;
  fields.title = BuildTitle(report);
  fields.labels = report.category == FeedbackCategory::Bug ? "bug" : "enhancement";

  const auto additionalContext = AdditionalContext(report, bundleFileName);
  const auto frame = BuildIssueBody(report, std::string(), additionalContext).size();
  const auto room = MaxIssueBodyLength > frame ? MaxIssueBodyLength - frame : 0;

  fields.body = BuildIssueBody(report, Shorten(report.description, room), additionalContext);

  return fields;
}

mitk::MailFields mitk::BuildMailFields(const FeedbackReport& report, const std::string& bundleFileName)
{
  ValidateBundleFileName(bundleFileName);

  std::string category = ToString(report.category);
  std::transform(category.begin(), category.end(), category.begin(),
    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  const auto head = "Hello MITK team,\n\nplease find my " + category + " attached as " + bundleFileName + ".\n\n";
  const auto tail = "\n\n" + BundleContents(report) + "\n";
  const auto frame = head.size() + tail.size();
  const auto room = MaxMailBodyLength > frame ? MaxMailBodyLength - frame : 0;

  MailFields fields;
  fields.recipient = GetFeedbackMailRecipient();
  fields.subject = "[MITK " + ToString(report.category) + "] " + BuildTitle(report);
  fields.body = head + Shorten(report.description, room) + tail;

  return fields;
}
