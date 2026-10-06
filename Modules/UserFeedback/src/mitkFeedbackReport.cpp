/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFeedbackReport.h>

#include <algorithm>

namespace
{
  bool IsAnyLogIncluded(const mitk::FeedbackReport& report)
  {
    return std::any_of(report.logs.begin(), report.logs.end(), [](const auto& log) { return log.included; });
  }

  bool IsAnyAttachmentIncluded(const mitk::FeedbackReport& report)
  {
    return std::any_of(report.attachments.begin(), report.attachments.end(),
      [](const auto& attachment) { return attachment.included; });
  }
}

bool mitk::FeedbackReport::NeedsSharingConfirmation() const
{
  return this->includeScreenshot || IsAnyLogIncluded(*this) || IsAnyAttachmentIncluded(*this);
}

bool mitk::FeedbackReport::CanUsePublicTracker() const
{
  if (IsAnyAttachmentIncluded(*this))
    return false;

  return this->publicSharingConfirmed || !(this->includeScreenshot || IsAnyLogIncluded(*this));
}

std::string mitk::GetSharingConfirmationStatement()
{
  return "I am permitted to share the included logs, screenshots, and crash dumps: doing so complies with "
         "the data-protection and other rules that apply to the data handled in the sessions concerned.";
}

std::string mitk::GetPublicSharingConfirmationStatement()
{
  return "I have checked the included log and screenshot, and they may be published: GitHub issues are "
         "public and can be read by anyone.";
}

std::string mitk::ToString(FeedbackCategory category)
{
  switch (category)
  {
    case FeedbackCategory::Bug:
      return "Bug report";

    case FeedbackCategory::Feature:
      return "Feature request";

    case FeedbackCategory::Idea:
      return "Idea";
  }

  return {};
}

std::string mitk::ToString(ScreenshotTarget target)
{
  switch (target)
  {
    case ScreenshotTarget::ApplicationWindow:
      return "Application window";

    case ScreenshotTarget::EditorArea:
      return "Editor area";
  }

  return {};
}
