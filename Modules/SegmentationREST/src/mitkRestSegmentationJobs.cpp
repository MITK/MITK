/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestSegmentationJobs.h>

#include <mitkException.h>
#include <mitkLabelSetImage.h>
#include <mitkRenderingManager.h>
#include <mitkRestHelpers.h>
#include <mitkRestSegmentation.h>
#include <mitkRestTime.h>
#include <mitkTimeNavigationController.h>
#include <mitkToolManager.h>

#include <QCoreApplication>
#include <QTimer>

namespace
{
  mitk::RestTimeSelection GetSelectedTime()
  {
    mitk::RestTimeSelection selection;
    selection.TimePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
    return selection;
  }
}

void mitk::FailRestJob(RestJob& job, std::exception_ptr error)
{
  try
  {
    std::rethrow_exception(error);
  }
  catch (const RestException& e)
  {
    job.Fail(e.what());
  }
  catch (const Exception& e)
  {
    job.Fail(e.GetDescription());
  }
  catch (const std::exception& e)
  {
    job.Fail(e.what());
  }
  catch (...)
  {
    job.Fail("An unknown error occurred.");
  }
}

std::exception_ptr mitk::MakeRestError(const std::string& message)
{
  try
  {
    mitkThrow() << message;
  }
  catch (...)
  {
    return std::current_exception();
  }
}

void mitk::WhenRestToolIdle(const SegWithPreviewTool::Pointer& tool, std::function<void()> then)
{
  if (!tool->IsUpdating() && !tool->IsRunningOffGUIThread())
  {
    then();
    return;
  }

  QTimer::singleShot(50, QCoreApplication::instance(), [tool, then = std::move(then)]() mutable {
    WhenRestToolIdle(tool, std::move(then));
  });
}

std::string mitk::GetRestConfirmationFailure(const SegWithPreviewTool& tool)
{
  using Outcome = SegWithPreviewTool::ConfirmationOutcome;
  const std::string name = tool.GetName();

  switch (tool.GetLastConfirmationOutcome())
  {
    case Outcome::Transferred:
      return {};
    case Outcome::Failed:
      return "The confirmation failed: " + tool.GetLastConfirmationError();
    case Outcome::Canceled:
      return name + " was canceled in the Workbench. Nothing was confirmed.";
    case Outcome::Abandoned:
      return name + " was deactivated before the result was confirmed. Nothing was confirmed.";
    case Outcome::None:
      break;
  }

  return "The confirmation did not end. Nothing was confirmed.";
}

void mitk::ThrowIfRestToolBusy(const SegWithPreviewTool* tool, const std::string& path)
{
  if (tool->IsUpdating() || tool->IsRunningOffGUIThread())
    ThrowRestError("BUSY", "Conflict", std::string(tool->GetName()) + " is still working. Wait for its job to end; "
      "after a cancel, GET /api/v1/workbench/tasks shows when the work has stopped.", 409, path);
}

void mitk::WhenRestStartEnded(const std::shared_ptr<RestJob>& job, std::function<bool()> isStarting, std::function<void()> then)
{
  if (job->HasEnded())
    return;

  if (!isStarting())
  {
    then();
    return;
  }

  QTimer::singleShot(50, QCoreApplication::instance(), [job, isStarting = std::move(isStarting), then = std::move(then)]() mutable {
    WhenRestStartEnded(job, std::move(isStarting), std::move(then));
  });
}

bool mitk::FailRestJobIfDeactivated(RestJob& job, ToolManager* toolManager, const Tool* tool)
{
  if (toolManager->GetActiveTool() == tool)
    return false;

  job.Fail(std::string(tool->GetName()) + " was deactivated while it worked.");
  return true;
}

nlohmann::json mitk::DescribeRestPreviewLabels(SegWithPreviewTool* tool, ToolManager* toolManager, bool measure)
{
  auto* preview = tool->GetPreviewSegmentation();

  if (preview == nullptr)
    return nlohmann::json::array();

  const auto* working = toolManager->GetWorkingData(0);
  const auto* segmentation = working != nullptr ? dynamic_cast<const MultiLabelSegmentation*>(working->GetData()) : nullptr;
  const auto group = segmentation != nullptr ? nlohmann::json(segmentation->GetActiveLayer()) : nlohmann::json(nullptr);

  auto labels = measure ? DescribeRestLabels(preview, GetSelectedTime()) : DescribeRestLabels(preview);

  for (auto& label : labels)
  {
    label.erase("value");

    if (!tool->GetTransfersAllPreviewGroups())
      label["group"] = group;
  }

  return labels;
}

std::uint64_t mitk::CountRestPreviewVoxels(SegWithPreviewTool* tool)
{
  auto* preview = tool->GetPreviewSegmentation();

  if (preview == nullptr)
    return 0;

  const auto timeSteps = GetSelectedTime().GetTimeSteps(preview);

  if (timeSteps.empty())
    return 0;

  std::uint64_t count = 0;

  for (MultiLabelSegmentation::GroupIndexType group = 0; group < preview->GetNumberOfGroups(); ++group)
  {
    try
    {
      for (const auto& [value, measure] : preview->MeasureLabels(group, timeSteps.front()))
        count += measure.VoxelCount;
    }
    catch (const Exception&)
    {
      // A group that a tool writes into is left out: the count never waits for the tool.
    }
  }

  return count;
}

void mitk::RestPromptOutcome::ReportToJob() const
{
  if (this->Job == nullptr || !this->Ended)
    return;

  if (this->Error != nullptr)
    FailRestJob(*this->Job, this->Error);
  else
    this->Job->Succeed(this->Result);
}
