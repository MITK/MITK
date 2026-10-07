/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFeedbackBundle.h>

#include <Poco/DateTime.h>
#include <Poco/Exception.h>
#include <Poco/LocalDateTime.h>
#include <Poco/Path.h>
#include <Poco/Zip/Compress.h>

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

namespace
{
  /** Paths in messages as UTF-8: path::string() throws on Windows for
   *  characters outside the ANSI code page. */
  std::string ToUtf8(const std::filesystem::path& path)
  {
    const auto utf8 = path.u8string();
    return std::string(utf8.begin(), utf8.end());
  }

  const std::string ReportEntry = "report.md";
  const std::string ScreenshotEntry = "screenshot.png";
  const std::string ManifestEntry = "manifest.txt";

  const std::string EditedLogMarker = "[This log excerpt was edited by the sender before sending.]\n";

  template <typename Item>
  bool AnyIncluded(const std::vector<Item>& items)
  {
    return std::any_of(items.begin(), items.end(), [](const auto& item) { return item.included; });
  }

  /** Archive names come from callers and may contain a crafted dump file
   *  name, so anything that could escape the archive root or that unzip
   *  tools on another platform could read as a path separator is rejected. */
  void ValidateArchiveName(const std::string& name, std::set<std::string>& usedNames)
  {
    if (name.empty() || name.find_first_of("\\:") != std::string::npos || name.front() == '/')
      mitkThrowException(mitk::FeedbackBundleException) << "Invalid archive name \"" << name << "\"";

    std::istringstream elements(name);
    std::string element;

    while (std::getline(elements, element, '/'))
    {
      if (element.empty() || element == "." || element == "..")
        mitkThrowException(mitk::FeedbackBundleException) << "Invalid archive name \"" << name << "\"";
    }

    if (!usedNames.insert(name).second)
      mitkThrowException(mitk::FeedbackBundleException) << "Duplicate archive name \"" << name << "\"";
  }

  void ValidateInput(const mitk::FeedbackReport& report)
  {
    std::set<std::string> usedNames = { ReportEntry, ScreenshotEntry, ManifestEntry };

    for (const auto& log : report.logs)
    {
      if (log.included)
        ValidateArchiveName(log.archiveName, usedNames);
    }

    for (const auto* files : { &report.images, &report.attachments })
    {
      for (const auto& file : *files)
      {
        if (!file.included)
          continue;

        ValidateArchiveName(file.archiveName, usedNames);

        std::error_code error;
        if (!std::filesystem::is_regular_file(file.sourcePath, error))
        {
          mitkThrowException(mitk::FeedbackBundleException)
            << "Missing file for archive entry \"" << file.archiveName << "\"";
        }
      }
    }
  }

  std::string LogContent(const mitk::FeedbackLog& log)
  {
    return log.edited ? EditedLogMarker + log.text : log.text;
  }

  class BundleWriter
  {
  public:
    explicit BundleWriter(std::ostream& out)
      : m_Zip(out, true)
    {
      const Poco::LocalDateTime now;
      m_Timestamp = Poco::DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    }

    void Add(const std::string& name, std::istream& content, std::uintmax_t size, const std::string& note = {})
    {
      m_Zip.addFile(content, m_Timestamp, Poco::Path(name, Poco::Path::PATH_UNIX));
      m_Manifest << name << '\t' << size << " bytes";

      if (!note.empty())
        m_Manifest << '\t' << note;

      m_Manifest << '\n';
    }

    void Add(const std::string& name, const std::string& content, const std::string& note = {})
    {
      std::istringstream stream(content);
      this->Add(name, stream, content.size(), note);
    }

    void Finish(const mitk::FeedbackReport& report)
    {
      std::ostringstream manifest;
      manifest << "MITK feedback bundle\n\n"
               << "Entries:\n" << m_Manifest.str() << '\n'
               << "Sharing confirmed: " << (report.sharingConfirmed ? "yes" : "no") << '\n'
               << "Public sharing confirmed: " << (report.publicSharingConfirmed ? "yes" : "no") << '\n';

      std::istringstream stream(manifest.str());
      m_Zip.addFile(stream, m_Timestamp, Poco::Path(ManifestEntry, Poco::Path::PATH_UNIX));
      m_Zip.close();
    }

  private:
    Poco::Zip::Compress m_Zip;
    Poco::DateTime m_Timestamp;
    std::ostringstream m_Manifest;
  };

  void WriteEntries(const mitk::FeedbackReport& report, std::ostream& out)
  {
    BundleWriter writer(out);

    writer.Add(ReportEntry, mitk::BuildReportText(report));

    for (const auto& log : report.logs)
    {
      if (log.included)
        writer.Add(log.archiveName, LogContent(log), log.edited ? "edited" : "");
    }

    if (report.includeScreenshot)
      writer.Add(ScreenshotEntry, std::string(report.screenshotPng.begin(), report.screenshotPng.end()));

    for (const auto* files : { &report.images, &report.attachments })
    {
      for (const auto& attachment : *files)
      {
        if (!attachment.included)
          continue;

        std::ifstream file(attachment.sourcePath, std::ios::binary);

        if (!file.is_open())
        {
          mitkThrowException(mitk::FeedbackBundleException)
            << "Cannot open file for archive entry \"" << attachment.archiveName << "\"";
        }

        writer.Add(attachment.archiveName, file, std::filesystem::file_size(attachment.sourcePath));
      }
    }

    writer.Finish(report);
  }
}

std::string mitk::MakeArchiveElement(std::string_view fileName)
{
  std::string element(fileName);

  // Byte-wise is safe on UTF-8: every replaced character is ASCII.
  for (auto& c : element)
  {
    if (static_cast<unsigned char>(c) < 0x20 || std::string_view("/\\:*?\"<>|").find(c) != std::string_view::npos)
      c = '_';
  }

  return element;
}

std::string mitk::BuildReportText(const FeedbackReport& report)
{
  std::ostringstream text;

  text << "# MITK Feedback Report\n\n"
       << "**Category:** " << ToString(report.category) << "\n\n"
       << "## Description\n\n"
       << report.description << "\n";

  if (AnyIncluded(report.environment))
  {
    text << "\n## Environment\n\n";

    for (const auto& item : report.environment)
    {
      if (item.included)
        text << "- " << item.label << ": " << item.value << '\n';
    }
  }

  if (AnyIncluded(report.logs))
  {
    text << "\n## Logs\n\n";

    for (const auto& log : report.logs)
    {
      if (!log.included)
        continue;

      text << "- " << log.sourceLabel << " (" << log.archiveName << ')';

      if (log.edited)
        text << ", edited by the sender";

      text << '\n';
    }
  }

  if (AnyIncluded(report.attachments))
  {
    text << "\n## Diagnostic Data\n\n"
         << "The environment above describes the session that filed this report. "
            "The diagnostic data may stem from other sessions, as noted for each item.\n\n";

    for (const auto& attachment : report.attachments)
    {
      if (attachment.included)
        text << "- " << attachment.description << " (" << attachment.archiveName << ")\n";
    }
  }

  if (report.includeScreenshot || AnyIncluded(report.images))
  {
    text << "\n## Screenshots\n\n";

    if (report.includeScreenshot)
      text << "- " << ToString(report.screenshotTarget) << " (" << ScreenshotEntry << ")\n";

    for (const auto& image : report.images)
    {
      if (image.included)
        text << "- " << image.description << " (" << image.archiveName << ")\n";
    }
  }

  if (report.sharingConfirmed || report.publicSharingConfirmed)
  {
    text << "\n## Sharing Confirmation\n\n";

    if (report.sharingConfirmed)
      text << "The sender confirmed: \"" << GetSharingConfirmationStatement() << "\"\n";

    if (report.publicSharingConfirmed)
      text << "The sender confirmed: \"" << GetPublicSharingConfirmationStatement() << "\"\n";
  }

  return text.str();
}

void mitk::WriteFeedbackBundle(const FeedbackReport& report, const std::filesystem::path& zipPath)
{
  ValidateInput(report);

  std::ofstream out(zipPath, std::ios::binary | std::ios::trunc);

  if (!out.is_open())
    mitkThrowException(FeedbackBundleException) << "Cannot create " << ToUtf8(zipPath);

  const auto removePartialZip = [&out, &zipPath]() {
    out.close();
    std::error_code error;
    std::filesystem::remove(zipPath, error);
  };

  try
  {
    WriteEntries(report, out);
    out.close();

    if (out.fail())
      mitkThrowException(FeedbackBundleException) << "Cannot write " << ToUtf8(zipPath);
  }
  catch (const FeedbackBundleException&)
  {
    removePartialZip();
    throw;
  }
  catch (const Poco::Exception& e)
  {
    removePartialZip();
    mitkThrowException(FeedbackBundleException) << "Cannot write " << ToUtf8(zipPath) << ": " << e.displayText();
  }
  catch (const std::exception& e)
  {
    removePartialZip();
    mitkThrowException(FeedbackBundleException) << "Cannot write " << ToUtf8(zipPath) << ": " << e.what();
  }
}
