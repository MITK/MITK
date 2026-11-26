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

  bool IsAnyIncluded(const std::vector<mitk::FeedbackAttachment>& attachments)
  {
    return std::any_of(attachments.begin(), attachments.end(), [](const auto& attachment) { return attachment.included; });
  }

  /** Whether a log, the screenshot, or an image is included: items that may
   *  show patient data, but that the user could review. */
  bool IsAnyReviewableItemIncluded(const mitk::FeedbackReport& report)
  {
    return report.includeScreenshot || IsAnyLogIncluded(report) || IsAnyIncluded(report.images);
  }
}

bool mitk::FeedbackReport::NeedsSharingConfirmation() const
{
  return IsAnyReviewableItemIncluded(*this) || IsAnyIncluded(this->attachments);
}

bool mitk::FeedbackReport::CanUsePublicTracker() const
{
  if (IsAnyIncluded(this->attachments))
    return false;

  return this->publicSharingConfirmed || !IsAnyReviewableItemIncluded(*this);
}

std::string mitk::GetSharingConfirmationStatement()
{
  return "I am permitted to share the included logs, screenshots, images, and crash dumps: doing so complies with "
         "the data-protection and other rules that apply to the data handled in the sessions concerned.";
}

std::string mitk::GetPublicSharingConfirmationStatement()
{
  return "I have checked the included logs and images, and they may be published: GitHub issues are "
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

std::string mitk::ToString(EnvironmentField field)
{
  switch (field)
  {
    case EnvironmentField::MitkVersion:
      return "MITK version";

    case EnvironmentField::Revision:
      return "Revision";

    case EnvironmentField::RevisionDescription:
      return "Revision description";

    case EnvironmentField::BuildType:
      return "Build type";

    case EnvironmentField::QtVersion:
      return "Qt version";

    case EnvironmentField::VtkVersion:
      return "VTK version";

    case EnvironmentField::ItkVersion:
      return "ITK version";

    case EnvironmentField::OperatingSystem:
      return "Operating system";

    case EnvironmentField::KernelType:
      return "Kernel type";

    case EnvironmentField::KernelVersion:
      return "Kernel version";

    case EnvironmentField::CpuArchitecture:
      return "CPU architecture";

    case EnvironmentField::UiLocale:
      return "UI locale";

    case EnvironmentField::GpuRenderer:
      return "GPU renderer";
  }

  return {};
}
