/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFeedbackHandoff.h>

#include <mitkExceptionMacro.h>

#include "mitkFeedbackMarkdown.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
  constexpr std::size_t MaxTitleLength = 80;

  // The caps below apply to the percent-encoded text, which is what ends up
  // in the URL and can be three times as long: GitHub rejects new-issue URLs
  // beyond about 8 kB, and some mail clients cut mailto: URLs beyond about
  // 2 kB. The rest of each URL stays well below the remainder.
  constexpr std::size_t MaxEncodedIssueBodyLength = 6000;
  constexpr std::size_t MaxEncodedMailLength = 1900;

  const std::string TruncationNote = " [...] (truncated; the full text is in report.md in the bundle)";

  /** Length of \p text percent-encoded as for a URL query, i.e. with every
   *  byte but the unreserved characters of RFC 3986 as three characters. */
  std::size_t EncodedLength(unsigned char c)
  {
    const bool unreserved = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
      c == '-' || c == '.' || c == '_' || c == '~';

    return unreserved ? 1 : 3;
  }

  std::size_t EncodedLength(const std::string& text)
  {
    std::size_t length = 0;

    for (const unsigned char c : text)
      length += EncodedLength(c);

    return length;
  }

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

  /** \p text, or its start with a truncation note, encoded at most
   *  \p maxEncodedLength long. If even the note does not fit, the start of
   *  \p text alone. */
  std::string Shorten(const std::string& text, std::size_t maxEncodedLength)
  {
    if (EncodedLength(text) <= maxEncodedLength)
      return text;

    const auto noteLength = EncodedLength(TruncationNote);
    const bool withNote = maxEncodedLength >= noteLength;
    const auto room = withNote ? maxEncodedLength - noteLength : maxEncodedLength;

    std::size_t length = 0;
    std::size_t encoded = 0;

    while (length < text.size())
    {
      const auto next = encoded + EncodedLength(static_cast<unsigned char>(text[length]));

      if (next > room)
        break;

      encoded = next;
      ++length;
    }

    const auto start = CutUtf8(text, length);
    return withNote ? start + TruncationNote : start;
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

  /** The included environment value of \p field, or empty. */
  std::string IncludedValue(const mitk::FeedbackReport& report, mitk::EnvironmentField field)
  {
    for (const auto& item : report.environment)
    {
      if (item.included && item.field == field)
        return item.value;
    }

    return {};
  }

  std::string OrNotIncluded(const std::string& value)
  {
    return value.empty() ? std::string("Not included.") : value;
  }

  /** How BundleContents refers to the included logs. A mail body counts them
   *  because the labels of many logs would take the room that the length cap
   *  leaves for the description. */
  enum class LogMention
  {
    ByLabel,
    ByCount
  };

  std::string BundleContents(const mitk::FeedbackReport& report, LogMention logMention)
  {
    std::ostringstream contents;
    contents << "The bundle contains the full report (report.md)";

    if (logMention == LogMention::ByLabel)
    {
      for (const auto& log : report.logs)
      {
        if (log.included)
          contents << ", the log excerpt \"" << log.sourceLabel << '"';
      }
    }
    else
    {
      const auto logs = std::count_if(report.logs.begin(), report.logs.end(),
        [](const auto& log) { return log.included; });

      if (logs == 1)
        contents << ", a log excerpt";
      else if (logs > 1)
        contents << ", " << logs << " log excerpts";
    }

    if (report.includeScreenshot)
      contents << ", a screenshot";

    const auto images = std::count_if(report.images.begin(), report.images.end(),
      [](const auto& image) { return image.included; });

    if (images == 1)
      contents << ", an image";
    else if (images > 1)
      contents << ", " << images << " images";

    const auto attachments = std::count_if(report.attachments.begin(), report.attachments.end(),
      [](const auto& attachment) { return attachment.included; });

    if (attachments == 1)
      contents << ", a diagnostic data file";
    else if (attachments > 1)
      contents << ", " << attachments << " diagnostic data files";

    contents << '.';
    return contents.str();
  }

  std::string ScreenshotsLine(const mitk::FeedbackReport& report)
  {
    const auto images = std::count_if(report.images.begin(), report.images.end(),
      [](const auto& image) { return image.included; });
    const auto total = images + (report.includeScreenshot ? 1 : 0);

    if (total == 0)
      return "None.";

    return total == 1 ? "One is in the bundle." : std::to_string(total) + " are in the bundle.";
  }

  /** Everything below the description, which is the part of the body that
   *  may be shortened. */
  std::string AdditionalContext(const mitk::FeedbackReport& report, const std::string& bundleFileName)
  {
    std::ostringstream context;
    context << "**Additional context**\n"
            << "Bundle file: " << bundleFileName << '\n'
            << BundleContents(report, LogMention::ByLabel) << '\n';

    bool hasEnvironment = false;

    for (const auto& item : report.environment)
    {
      if (!item.included)
        continue;

      if (!hasEnvironment)
        context << "\nEnvironment:\n";

      hasEnvironment = true;
      context << "- " << mitk::ToString(item.field) << ": " << item.value << '\n';
    }

    return context.str();
  }

  std::string BuildBugBody(const mitk::FeedbackReport& report, const std::string& description,
    const std::string& additionalContext)
  {
    auto version = IncludedValue(report, mitk::EnvironmentField::MitkVersion);
    const auto revision = IncludedValue(report, mitk::EnvironmentField::RevisionDescription);

    if (!version.empty() && !revision.empty())
      version += " (" + revision + ')';

    std::ostringstream body;
    body << "**Describe the bug**\n" << description << "\n\n"
         << "**MITK version**\n" << OrNotIncluded(version) << "\n\n"
         << "**Operating system**\n" << OrNotIncluded(IncludedValue(report, mitk::EnvironmentField::OperatingSystem)) << "\n\n"
         << "**To Reproduce**\nSee the description above.\n\n"
         << "**Expected behavior**\nSee the description above.\n\n"
         << "**Screenshots**\n" << ScreenshotsLine(report) << "\n\n"
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
    const std::string& bundleFileName, const std::string& additionalContext)
  {
    // On top, because a prefilled issue cannot carry the bundle and the
    // user may not read down to the additional context before submitting.
    const auto reminder = "**Before submitting:** drag " + bundleFileName +
      " from the folder MITK opened into this text box to attach it, then delete this note.\n\n";

    return reminder + (report.category == mitk::FeedbackCategory::Bug
      ? BuildBugBody(report, description, additionalContext)
      : BuildFeatureBody(description, additionalContext));
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
  const auto frame = EncodedLength(BuildIssueBody(report, std::string(), bundleFileName, additionalContext));
  const auto room = MaxEncodedIssueBodyLength > frame ? MaxEncodedIssueBodyLength - frame : 0;

  fields.body = BuildIssueBody(report, Shorten(EscapeMarkdownHtml(report.description), room), bundleFileName,
    additionalContext);

  return fields;
}

mitk::MailFields mitk::BuildMailFields(const FeedbackReport& report, const std::string& bundleFileName)
{
  ValidateBundleFileName(bundleFileName);

  std::string category = ToString(report.category);
  std::transform(category.begin(), category.end(), category.begin(),
    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  // A mailto: URL cannot carry the bundle, so the body reminds the user to
  // attach it.
  const auto head = "[Before sending: attach " + bundleFileName +
    " from the folder MITK opened, then delete this note.]\n\n"
    "Hello MITK team,\n\nplease find my " + category + " attached as " + bundleFileName + ".\n\n";
  const auto tail = "\n\n" + BundleContents(report, LogMention::ByCount) + "\n";
  MailFields fields;
  fields.recipient = GetFeedbackMailRecipient();
  fields.subject = "[MITK " + ToString(report.category) + "] " + BuildTitle(report);

  const auto frame = EncodedLength(fields.subject) + EncodedLength(head) + EncodedLength(tail);
  const auto room = MaxEncodedMailLength > frame ? MaxEncodedMailLength - frame : 0;

  fields.body = head + Shorten(report.description, room) + tail;

  return fields;
}
