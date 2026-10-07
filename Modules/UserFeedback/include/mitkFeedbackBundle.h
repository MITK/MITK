/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFeedbackBundle_h
#define mitkFeedbackBundle_h

#include <MitkUserFeedbackExports.h>

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkFeedbackReport.h>

#include <filesystem>
#include <string>

namespace mitk
{
  /** \brief Thrown by WriteFeedbackBundle() when the bundle cannot be written
   *  (invalid archive name, missing attachment source, file, or zip failure). */
  class MITKUSERFEEDBACK_EXPORT FeedbackBundleException : public Exception
  {
  public:
    mitkExceptionClassMacro(FeedbackBundleException, Exception);
  };

  /** \brief Render the human-readable report as shown in the preview and
   *  written to report.md: category, description, included environment
   *  items, included logs (by sourceLabel, with an edited note), included
   *  attachments (by description), the screenshot target, and the sharing
   *  confirmations when given. Deterministic. */
  MITKUSERFEEDBACK_EXPORT std::string BuildReportText(const FeedbackReport& report);

  /**
   * \brief Write \p report to a .zip at \p zipPath.
   *
   * Entries, in this order:
   * - report.md: always, BuildReportText()
   * - each included log under its archiveName; its first line marks it as
   *   edited if it was
   * - screenshot.png: only if includeScreenshot
   * - each included image, then each included attachment, under its
   *   archiveName, copied verbatim
   * - manifest.txt: always; the entries with their sizes, edited flags, and
   *   whether the sharing confirmations were given
   *
   * Archive names must be relative, must not contain "..", and must be
   * unique. Throws FeedbackBundleException on failure; a partially written
   * zip is removed. Reads only \p report and the attachment files, so it may
   * run off the UI thread.
   */
  MITKUSERFEEDBACK_EXPORT void WriteFeedbackBundle(const FeedbackReport& report, const std::filesystem::path& zipPath);
}

#endif
