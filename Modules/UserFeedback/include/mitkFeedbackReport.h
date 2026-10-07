/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFeedbackReport_h
#define mitkFeedbackReport_h

#include <MitkUserFeedbackExports.h>

#include <filesystem>
#include <string>
#include <vector>

namespace mitk
{
  /** \brief What the user is reporting. Bug maps to the GitHub "bug" label;
   *  Feature and Idea both map to "enhancement" (the tracker has no idea label). */
  enum class FeedbackCategory
  {
    Bug,
    Feature,
    Idea
  };

  /** \brief What a screenshot shows. */
  enum class ScreenshotTarget
  {
    ApplicationWindow,
    EditorArea
  };

  /** \brief The environment facts a report can carry; see ToString() for the
   *  display label. */
  enum class EnvironmentField
  {
    MitkVersion,
    Revision,
    RevisionDescription,
    BuildType,
    QtVersion,
    VtkVersion,
    ItkVersion,
    OperatingSystem,
    KernelType,
    KernelVersion,
    CpuArchitecture,
    UiLocale,
    GpuRenderer
  };

  /** \brief One auto-detected environment fact; include-or-omit, never free-edited. */
  struct FeedbackItem
  {
    EnvironmentField field;
    std::string value;
    bool included = true;
  };

  /** \brief One log excerpt offered for inclusion. */
  struct FeedbackLog
  {
    std::string archiveName; /**< Path inside the bundle, e.g. "logs/current-session.log". */
    std::string sourceLabel; /**< Shown to the user and in report.md; never a path. */
    std::string text;        /**< As shown (and possibly edited) in the dialog. */
    bool included = false;   /**< Opt-in. */
    bool edited = false;     /**< True if the user changed the text. */
  };

  /** \brief A file copied verbatim into the bundle (e.g. a crash dump or an
   *  image the user picked). */
  struct FeedbackAttachment
  {
    std::filesystem::path sourcePath; /**< Read when the bundle is written; never emitted. */
    std::string archiveName;          /**< Path inside the bundle, e.g. "dumps/<name>.dmp". */
    std::string description;          /**< One line for report.md. */
    bool included = true;
  };

  /**
   * \brief The complete, user-reviewed payload.
   *
   * Everything a hand-off or the bundle emits is derived solely from this
   * object, so what the user reviewed is what is sent. Holds no Qt types.
   */
  struct MITKUSERFEEDBACK_EXPORT FeedbackReport
  {
    FeedbackCategory category = FeedbackCategory::Bug;
    std::string description; /**< Required (non-empty). */
    std::vector<FeedbackItem> environment;
    std::vector<FeedbackLog> logs;

    /** Files the user cannot review, such as crash dumps; they keep the
     *  report off public trackers. */
    std::vector<FeedbackAttachment> attachments;

    /** Image files the user picked and reviewed; like the screenshot, they
     *  may go to a public tracker once publicSharingConfirmed is set. */
    std::vector<FeedbackAttachment> images;

    bool includeScreenshot = false;
    ScreenshotTarget screenshotTarget = ScreenshotTarget::ApplicationWindow;
    std::vector<unsigned char> screenshotPng; /**< PNG bytes from the UI layer. */

    /** The user confirmed GetSharingConfirmationStatement(). */
    bool sharingConfirmed = false;

    /** The user confirmed GetPublicSharingConfirmationStatement(). */
    bool publicSharingConfirmed = false;

    /** \brief True if any included item may carry patient data (an attachment,
     *  a log, the screenshot, or an image); then sharingConfirmed is
     *  required. */
    bool NeedsSharingConfirmation() const;

    /** \brief Whether the report may go to a public tracker: no attachment is
     *  included (attachments such as crash dumps cannot be reviewed by the
     *  user), and if a log, the screenshot, or an image is included,
     *  publicSharingConfirmed is set. */
    bool CanUsePublicTracker() const;
  };

  /** \brief The statement the user confirms with FeedbackReport::sharingConfirmed,
   *  worded for display and recorded in the bundle. */
  MITKUSERFEEDBACK_EXPORT std::string GetSharingConfirmationStatement();

  /** \brief The statement the user confirms with
   *  FeedbackReport::publicSharingConfirmed. */
  MITKUSERFEEDBACK_EXPORT std::string GetPublicSharingConfirmationStatement();

  /** \brief Display name of \p category, e.g. "Bug report". */
  MITKUSERFEEDBACK_EXPORT std::string ToString(FeedbackCategory category);

  /** \brief Display name of \p target, e.g. "Application window". */
  MITKUSERFEEDBACK_EXPORT std::string ToString(ScreenshotTarget target);

  /** \brief Display label of \p field, e.g. "MITK version". */
  MITKUSERFEEDBACK_EXPORT std::string ToString(EnvironmentField field);
}

#endif
