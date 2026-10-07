/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFeedbackHandoff_h
#define mitkFeedbackHandoff_h

#include <MitkUserFeedbackExports.h>

#include <mitkFeedbackReport.h>

#include <string>

namespace mitk
{
  /** \brief Fields of a GitHub "new issue" before URL encoding. */
  struct GitHubIssueFields
  {
    std::string title;  /**< First line of the description, capped. */
    std::string body;   /**< Follows the repository's issue template; its description is
                             shortened so that the percent-encoded body fits a URL. */
    std::string labels; /**< "bug" or "enhancement"; GitHub may ignore it. */
  };

  /** \brief Fields of a mailto: hand-off before URL encoding. */
  struct MailFields
  {
    std::string recipient;
    std::string subject;
    std::string body;
  };

  /** \brief The address diagnostic reports go to. */
  MITKUSERFEEDBACK_EXPORT std::string GetFeedbackMailRecipient();

  /** \brief Fields for a public GitHub issue about \p report.
   *
   *  \p bundleFileName must be a bare file name: an absolute path would leak
   *  the OS user name into a public tracker. Logs and screenshots never go
   *  into the fields; the body only names those the bundle contains. Throws
   *  mitk::Exception if !report.CanUsePublicTracker(), so the gate holds
   *  regardless of the UI, or if \p bundleFileName is not a bare file name. */
  MITKUSERFEEDBACK_EXPORT GitHubIssueFields BuildGitHubIssueFields(const FeedbackReport& report,
                                                                   const std::string& bundleFileName);

  /** \brief Fields for an email about \p report to GetFeedbackMailRecipient(),
   *  with a body that asks to attach \p bundleFileName (a bare file name;
   *  throws mitk::Exception otherwise). The description is shortened so that
   *  the percent-encoded subject and body stay below the length at which
   *  some mail clients cut mailto: URLs. */
  MITKUSERFEEDBACK_EXPORT MailFields BuildMailFields(const FeedbackReport& report, const std::string& bundleFileName);
}

#endif
