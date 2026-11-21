/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestPythonSegmentation.h>

#include <mitkActionOriginScope.h>
#include <mitkErrorResponse.h>
#include <mitkProgressTask.h>
#include <mitkRestSegmentationJobs.h>

#include <QCoreApplication>
#include <QTimer>

namespace
{
  /** Why a session was lost, in the words that tell an agent whether to start it again. */
  std::string DescribeSessionLoss(const mitk::nnInteractiveTool::SessionLoss& loss)
  {
    using Cause = mitk::nnInteractiveTool::SessionLoss::Cause;

    switch (loss.Reason)
    {
      case Cause::Cancelled:
        return loss.ByProgram
          ? "nnInteractive: cancelled, which ended its session."
          : "nnInteractive was canceled in the Workbench, which ended its session.";
      case Cause::RemoteConnection:
        return "nnInteractive lost its session on the server: " + loss.Details;
      default:
        return "nnInteractive stopped unexpectedly: " + loss.Details;
    }
  }

  /**
   * Waits until TotalSegmentator has started, or failed to, whoever started
   * it, as the tool tells that only by events. Deletes itself afterwards.
   */
  class TotalSegmentatorStartListener
  {
  public:
    static void Start(mitk::TotalSegmentatorTool* tool, std::function<void(std::exception_ptr)> done)
    {
      if (tool->IsInitialized())
      {
        done(nullptr);
        return;
      }

      // Listens before the start, which can fail at once.
      auto* listener = new TotalSegmentatorStartListener(tool, std::move(done));

      try
      {
        // A start that runs already has its task; another one would only flash.
        tool->Initialize(tool->IsInitializing()
          ? nullptr
          : std::make_shared<mitk::ProgressTask>("Starting TotalSegmentator", mitk::ProgressTask::Indeterminate, true));
      }
      catch (...)
      {
        // No event follows, so nothing else would end the listener.
        listener->End(std::current_exception());
      }
    }

  private:
    TotalSegmentatorStartListener(mitk::TotalSegmentatorTool* tool, std::function<void(std::exception_ptr)> done)
      : m_Tool(tool),
        m_Done(std::move(done)),
        m_Origin(mitk::ActionOriginScope::GetCurrent())
    {
      m_Tool->InitializedEvent += mitk::MessageDelegate<TotalSegmentatorStartListener>(this, &TotalSegmentatorStartListener::OnInitialized);
      m_Tool->InitializationFailedEvent += mitk::MessageDelegate1<TotalSegmentatorStartListener, std::string>(this, &TotalSegmentatorStartListener::OnInitializationFailed);
    }

    void OnInitialized()
    {
      this->End(nullptr);
    }

    void OnInitializationFailed(std::string message)
    {
      this->End(mitk::MakeRestError(message.empty() ? std::string("Starting TotalSegmentator was canceled in the Workbench.") : message));
    }

    // Not from within the event, which goes on to its other listeners, and
    // after which what follows may run the tool.
    void End(std::exception_ptr error)
    {
      // The events still reach the listener until the queued call has run.
      if (m_Ended)
        return;

      m_Ended = true;

      QTimer::singleShot(0, QCoreApplication::instance(), [this, error]() {
        m_Tool->InitializedEvent -= mitk::MessageDelegate<TotalSegmentatorStartListener>(this, &TotalSegmentatorStartListener::OnInitialized);
        m_Tool->InitializationFailedEvent -= mitk::MessageDelegate1<TotalSegmentatorStartListener, std::string>(this, &TotalSegmentatorStartListener::OnInitializationFailed);

        const auto done = std::move(m_Done);
        const auto origin = m_Origin;
        delete this;

        // What done starts, like the run of a REST client, is the caller's,
        // as if it had followed the start at once.
        const mitk::ActionOriginScope scope(origin);
        done(error);
      });
    }

    mitk::TotalSegmentatorTool::Pointer m_Tool;
    std::function<void(std::exception_ptr)> m_Done;
    const std::string m_Origin;
    bool m_Ended = false;
  };
}

nlohmann::json mitk::DescribeRestNNInteractive(nnInteractiveTool* tool)
{
  const auto supported = tool->GetSupportedInteractions();
  auto prompts = nlohmann::json::array();

  if (supported.Point)
    prompts.push_back("point");

  if (supported.Box)
    prompts.push_back("box");

  if (supported.Scribble)
    prompts.push_back("scribble");

  if (supported.Lasso)
    prompts.push_back("lasso");

  const bool running = tool->IsSessionRunning();

  return {
    { "session_running", running },
    { "session_starting", tool->IsSessionStarting() },
    { "remote", tool->IsRemoteSession() },
    { "prompt_types", prompts },
    { "can_undo", tool->CanUndo() },
    { "auto_zoom", tool->GetAutoZoom() },
    { "auto_refine", tool->GetAutoRefine() },
    { "preview_voxels", running ? CountRestPreviewVoxels(tool) : 0 }
  };
}

mitk::nnInteractive::InteractionType mitk::ParseRestInteractionType(const nlohmann::json& body, const std::string& path)
{
  const auto type = body.value("type", std::string());

  if (type == "point")
    return nnInteractive::InteractionType::Point;

  if (type == "box")
    return nnInteractive::InteractionType::Box;

  if (type == "scribble")
    return nnInteractive::InteractionType::Scribble;

  if (type == "lasso")
    return nnInteractive::InteractionType::Lasso;

  throw RestException(ErrorResponse::InvalidRequest("\"type\" must be point, box, scribble or lasso.", path));
}

void mitk::SucceedRestJobWhenSessionSeeded(const std::shared_ptr<RestJob>& job, const nnInteractiveTool::Pointer& tool)
{
  QTimer::singleShot(0, QCoreApplication::instance(), [job, tool]() {
    WhenRestToolIdle(tool, [job, tool]() {
      // After the teardown that a session asked to end posts once its work has stopped.
      QTimer::singleShot(0, QCoreApplication::instance(), [job, tool]() {
        if (const auto loss = tool->GetLastSessionLoss(); loss.has_value())
        {
          job->Fail(DescribeSessionLoss(*loss));
          return;
        }

        if (!tool->IsSessionRunning() || tool->IsSessionEnding())
        {
          job->Fail("The nnInteractive session ended before it was ready, as it was ended or the tool deactivated "
            "meanwhile. Start it again if it is still needed.");
          return;
        }

        job->Succeed(DescribeRestNNInteractive(tool));
      });
    });
  });
}

nlohmann::json mitk::DescribeRestVoxTell(VoxTellTool* tool, ToolManager* toolManager, bool installed, bool measure)
{
  return {
    { "installed", installed },
    { "model_loaded", tool->IsModelLoaded() },
    { "model_loading", tool->IsInitializing() },
    { "prompts", tool->GetPrompts() },
    { "prompts_without_result", tool->GetPromptsWithoutResult() },
    { "error", tool->GetLastErrorMessage().empty() ? nlohmann::json() : nlohmann::json(tool->GetLastErrorMessage()) },
    { "preview_labels", DescribeRestPreviewLabels(tool, toolManager, measure) }
  };
}

std::string mitk::ToRestJson(TotalSegmentatorTool::Speed speed)
{
  switch (speed)
  {
    case TotalSegmentatorTool::Speed::Fast:
      return "fast";
    case TotalSegmentatorTool::Speed::Fastest:
      return "fastest";
    default:
      return "standard";
  }
}

mitk::TotalSegmentatorTool::Speed mitk::ParseRestSpeed(const nlohmann::json& json, const std::string& path)
{
  const auto speed = json.is_string() ? json.get<std::string>() : std::string();

  if (speed == "standard")
    return TotalSegmentatorTool::Speed::Standard;

  if (speed == "fast")
    return TotalSegmentatorTool::Speed::Fast;

  if (speed == "fastest")
    return TotalSegmentatorTool::Speed::Fastest;

  throw RestException(ErrorResponse::InvalidRequest("\"speed\" must be standard, fast or fastest.", path));
}

nlohmann::json mitk::DescribeRestTotalSegmentatorTask(const TotalSegmentatorTool::TaskInfo& task, bool withClasses)
{
  auto speeds = nlohmann::json::array();

  for (const auto& speed : task.Speeds)
  {
    speeds.push_back({
      { "name", ToRestJson(speed.Value) },
      { "resolution_mm", speed.Resolution > 0 ? nlohmann::json(speed.Resolution) : nlohmann::json() }
    });
  }

  nlohmann::json json = {
    { "name", task.Name },
    { "modality", task.MR ? "MR" : "CT" },
    { "licensed", task.Licensed },
    { "available", !task.Licensed || TotalSegmentatorTool::HasLicense() },
    { "speeds", speeds },
    { "roi_subset", task.SupportsRoiSubset },
    { "class_count", task.Classes.size() }
  };

  if (withClasses)
  {
    auto classes = nlohmann::json::array();

    for (const auto& [value, name] : task.Classes)
      classes.push_back(name);

    json["classes"] = classes;
  }

  return json;
}

nlohmann::json mitk::DescribeRestTotalSegmentator(TotalSegmentatorTool* tool, ToolManager* toolManager, bool withTasks, bool measure)
{
  nlohmann::json state = {
    { "installed", TotalSegmentatorTool::IsInstalled() },
    { "initialized", tool->IsInitialized() },
    { "has_license", TotalSegmentatorTool::HasLicense() },
    { "device", TotalSegmentatorTool::GetDevice() },
    { "task", tool->GetTask() },
    { "speed", ToRestJson(tool->GetSpeed()) },
    { "roi_subset", tool->GetRoiSubset() },
    { "running", tool->IsUpdating() },
    { "error", tool->GetLastErrorMessage().empty() ? nlohmann::json() : nlohmann::json(tool->GetLastErrorMessage()) },
    { "preview_labels", DescribeRestPreviewLabels(tool, toolManager, measure) }
  };

  if (withTasks && tool->IsInitialized())
  {
    auto tasks = nlohmann::json::array();

    for (const auto& task : tool->GetTasks())
      tasks.push_back(DescribeRestTotalSegmentatorTask(task, false));

    state["tasks"] = tasks;
  }

  return state;
}

void mitk::StartRestTotalSegmentator(TotalSegmentatorTool* tool, std::function<void(std::exception_ptr)> done)
{
  TotalSegmentatorStartListener::Start(tool, std::move(done));
}
