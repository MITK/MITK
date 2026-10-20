/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationRestRoutes.h"

#include <mitkSegmentationPluginConfig.h>

#include <mitkCoreServices.h>
#include <mitkErrorResponse.h>
#include <mitkImageReadAccessor.h>
#include <mitkINodeSelectionService.h>
#include <mitkIProgressService.h>
#include <mitkIRestServerService.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkRenderingManager.h>
#include <mitkRestGuiThread.h>
#include <mitkRestJob.h>
#include <mitkRestPoints.h>
#include <mitkRestTime.h>
#include <mitkSegChangeOperationApplier.h>
#include <mitkSegWithPreviewTool.h>
#include <mitkTimeNavigationController.h>
#include <mitkToolManagerProvider.h>

#if MITK_HAS_PYTHON
#include <QmitkVenvProcess.h>
#include <mitkBaseRenderer.h>
#include <mitkProgressTask.h>
#include <mitkPythonHelper.h>
#include <mitkTotalSegmentatorInstall.h>
#include <mitkTotalSegmentatorTool.h>
#include <mitkVoxTellInstall.h>
#include <mitkVoxTellTool.h>
#include <mitknnInteractiveTool.h>
#endif

#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryPlatformUI.h>

#include <QCoreApplication>
#include <QTimer>

#include <algorithm>
#include <map>
#include <set>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>

namespace
{
  const std::string ViewId = "org.mitk.views.segmentation";

  [[noreturn]] void Throw(const std::string& code, const std::string& title, const std::string& detail, int status, const std::string& path)
  {
    throw mitk::RestException(mitk::ErrorResponse::Create(code, title, detail, status, path));
  }

  mitk::ToolManager* GetToolManager()
  {
    return mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  }

  berry::IWorkbenchPage::Pointer GetPage(const std::string& path)
  {
    auto* workbench = berry::PlatformUI::GetWorkbench();
    auto window = workbench->GetActiveWorkbenchWindow();

    if (window.IsNull() && !workbench->GetWorkbenchWindows().isEmpty())
      window = workbench->GetWorkbenchWindows().front();

    if (window.IsNull() || window->GetActivePage().IsNull())
      Throw("WORKBENCH_NOT_AVAILABLE", "Service Unavailable", "The Workbench has no window.", 503, path);

    return window->GetActivePage();
  }

  bool IsViewOpen(const std::string& path)
  {
    return GetPage(path)->FindView(QString::fromStdString(ViewId)).IsNotNull();
  }

  /**
   * Opens the view, or brings it to the front of its tab group, without
   * activating it. The view sets up the tools and owns the selection of image
   * and segmentation, and the user sees there what a client changes.
   */
  void ShowView(const std::string& path)
  {
    GetPage(path)->ShowView(QString::fromStdString(ViewId), QString(), berry::IWorkbenchPage::VIEW_VISIBLE);
  }

  nlohmann::json DescribeNode(mitk::DataNode* node, const mitk::IRestServerService& server)
  {
    if (node == nullptr)
      return nullptr;

    return { { "uid", server.GetNodeUid(node) }, { "name", node->GetName() } };
  }

  std::string ToHex(const mitk::Color& color)
  {
    char text[8];
    std::snprintf(text, sizeof(text), "#%02x%02x%02x",
      static_cast<int>(std::lround(color[0] * 255)),
      static_cast<int>(std::lround(color[1] * 255)),
      static_cast<int>(std::lround(color[2] * 255)));

    return text;
  }

  mitk::Color ParseColor(const nlohmann::json& json, const std::string& path)
  {
    const auto text = json.is_string() ? json.get<std::string>() : std::string();

    if (text.size() != 7 || text[0] != '#' ||
        !std::all_of(text.begin() + 1, text.end(), [](unsigned char c) { return std::isxdigit(c) != 0; }))
    {
      throw mitk::RestException(mitk::ErrorResponse::InvalidRequest("A color must be written like \"#ff8800\".", path));
    }

    mitk::Color color;

    for (int i = 0; i < 3; ++i)
      color[i] = std::stoi(text.substr(1 + 2 * i, 2), nullptr, 16) / 255.0f;

    return color;
  }

  mitk::RestTimeSelection GetSelectedTime()
  {
    mitk::RestTimeSelection selection;
    selection.TimePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
    return selection;
  }

  /** Voxels per label value in a group image, empty while a tool writes into it. */
  std::vector<std::uint64_t> CountVoxels(const mitk::Image* groupImage, mitk::TimeStepType timeStep)
  {
    std::vector<std::uint64_t> counts;

    if (groupImage == nullptr || groupImage->GetPixelType().GetComponentType() != itk::IOComponentEnum::USHORT)
      return counts;

    try
    {
      // Never wait in the GUI thread for a tool that writes into the segmentation.
      mitk::ImageReadAccessor accessor(mitk::Image::ConstPointer(groupImage), groupImage->GetVolumeData(timeStep), mitk::ImageAccessorBase::ExceptionIfLocked);
      const auto* pixels = static_cast<const mitk::Label::PixelType*>(accessor.GetData());
      const auto size = static_cast<std::size_t>(groupImage->GetDimension(0)) * groupImage->GetDimension(1) * groupImage->GetDimension(2);

      counts.assign(std::numeric_limits<mitk::Label::PixelType>::max() + 1, 0);

      for (std::size_t i = 0; i < size; ++i)
        ++counts[pixels[i]];
    }
    catch (const mitk::Exception&)
    {
      counts.clear();
    }

    return counts;
  }

  nlohmann::json DescribeLabel(const mitk::Label* label, mitk::MultiLabelSegmentation::GroupIndexType group)
  {
    return {
      { "value", label->GetValue() },
      { "name", label->GetName() },
      { "color", ToHex(label->GetColor()) },
      { "visible", label->GetVisible() },
      { "locked", label->GetLocked() },
      { "group", group }
    };
  }

  /**
   * The labels with their voxels and volumes at the time point of the
   * selection, or with time_steps for all of them.
   */
  nlohmann::json DescribeLabels(mitk::MultiLabelSegmentation* segmentation, const mitk::RestTimeSelection& selection)
  {
    auto labels = nlohmann::json::array();
    const auto timeSteps = selection.GetTimeSteps(segmentation);

    for (mitk::MultiLabelSegmentation::GroupIndexType group = 0; group < segmentation->GetNumberOfGroups(); ++group)
    {
      const auto* groupImage = segmentation->GetGroupImage(group);
      const auto spacing = groupImage->GetGeometry()->GetSpacing();
      const auto voxelMilliliters = spacing[0] * spacing[1] * spacing[2] / 1000.0;

      std::vector<std::vector<std::uint64_t>> counts;

      for (const auto timeStep : timeSteps)
        counts.push_back(CountVoxels(groupImage, timeStep));

      const auto addVolume = [voxelMilliliters](nlohmann::json& json, const std::vector<std::uint64_t>* stepCounts, mitk::MultiLabelSegmentation::LabelValueType value) {
        if (stepCounts == nullptr || stepCounts->empty())
        {
          json["voxels"] = nullptr;
          return;
        }

        json["voxels"] = (*stepCounts)[value];
        json["volume_ml"] = std::round((*stepCounts)[value] * voxelMilliliters * 100.0) / 100.0;
      };

      for (const auto value : segmentation->GetLabelValuesByGroup(group))
      {
        const auto label = segmentation->GetLabel(value);

        if (label.IsNull())
          continue;

        auto json = DescribeLabel(label, group);

        if (selection.All)
        {
          auto steps = nlohmann::json::array();

          for (std::size_t i = 0; i < timeSteps.size(); ++i)
          {
            auto step = mitk::DescribeRestTimeStep(segmentation, timeSteps[i]);
            addVolume(step, &counts[i], value);
            steps.push_back(std::move(step));
          }

          json["time_steps"] = std::move(steps);
        }
        else
        {
          addVolume(json, counts.empty() ? nullptr : &counts.front(), value);
        }

        labels.push_back(std::move(json));
      }
    }

    return labels;
  }

  /** The labels with their voxels and volumes at the selected time point. */
  nlohmann::json DescribeLabels(mitk::MultiLabelSegmentation* segmentation)
  {
    return DescribeLabels(segmentation, GetSelectedTime());
  }

  nlohmann::json DescribeGroup(mitk::MultiLabelSegmentation* segmentation, mitk::MultiLabelSegmentation::GroupIndexType group)
  {
    auto labels = nlohmann::json::array();

    for (const auto value : segmentation->GetLabelValuesByGroup(group))
    {
      if (const auto label = segmentation->GetLabel(value); label.IsNotNull())
        labels.push_back(DescribeLabel(label, group));
    }

    return {
      { "index", group },
      { "name", segmentation->GetGroupName(group) },
      { "display_name", mitk::LabelSetImageHelper::CreateDisplayGroupName(segmentation, group) },
      { "labels", labels }
    };
  }

  nlohmann::json DescribeGroups(mitk::MultiLabelSegmentation* segmentation)
  {
    auto groups = nlohmann::json::array();

    for (mitk::MultiLabelSegmentation::GroupIndexType group = 0; group < segmentation->GetNumberOfGroups(); ++group)
      groups.push_back(DescribeGroup(segmentation, group));

    return groups;
  }

  /** Whether a tool that runs in a Python environment of its own has it; nothing for other tools. */
  std::optional<bool> IsInstalled(const mitk::Tool* tool)
  {
#if MITK_HAS_PYTHON
    std::string venv;

    if (const auto* nnInteractive = dynamic_cast<const mitk::nnInteractiveTool*>(tool); nnInteractive != nullptr)
    {
      venv = nnInteractive->GetVirtualEnvName();
    }
    else if (dynamic_cast<const mitk::VoxTellTool*>(tool) != nullptr)
    {
      venv = mitk::VoxTell::VENV_NAME;
    }
    else if (dynamic_cast<const mitk::TotalSegmentatorTool*>(tool) != nullptr)
    {
      venv = mitk::TotalSegmentator::VENV_NAME;
    }
    else
    {
      return std::nullopt;
    }

    return !mitk::PythonHelper::GetVirtualEnvExecutablePath(venv).empty();
#else
    (void)tool;
    return std::nullopt;
#endif
  }

  nlohmann::json DescribeSession(const mitk::IRestServerService& server, const std::string& path)
  {
    auto* toolManager = GetToolManager();
    auto* reference = toolManager->GetReferenceData(0);
    auto* working = toolManager->GetWorkingData(0);
    auto* segmentation = working != nullptr ? dynamic_cast<mitk::MultiLabelSegmentation*>(working->GetData()) : nullptr;
    const auto* activeLabel = segmentation != nullptr ? segmentation->GetActiveLabel() : nullptr;
    const auto* activeTool = toolManager->GetActiveTool();

    auto tools = nlohmann::json::array();

    for (const auto& tool : toolManager->GetTools())
    {
      nlohmann::json entry = {
        { "name", tool->GetName() },
        { "usable", tool->CanHandle(reference != nullptr ? reference->GetData() : nullptr, working != nullptr ? working->GetData() : nullptr) }
      };

      if (const auto installed = IsInstalled(tool); installed.has_value())
        entry["installed"] = *installed;

      tools.push_back(std::move(entry));
    }

    const auto describeWithTime = [&server](mitk::DataNode* node) {
      auto json = DescribeNode(node, server);

      if (node != nullptr && node->GetData() != nullptr)
        json["time_steps"] = mitk::DescribeRestTimeSteps(node->GetData());

      return json;
    };

    return {
      { "view_open", IsViewOpen(path) },
      { "reference", describeWithTime(reference) },
      { "segmentation", describeWithTime(working) },
      { "active_label", activeLabel != nullptr
          ? nlohmann::json{ { "value", activeLabel->GetValue() }, { "name", activeLabel->GetName() } }
          : nlohmann::json() },
      { "tool", activeTool != nullptr ? nlohmann::json(activeTool->GetName()) : nlohmann::json() },
      { "tools", tools }
    };
  }

  /** Selects a node in one of the view's node selectors, as the user would. */
  void SelectInView(const std::string& selector, mitk::DataNode* node)
  {
    std::vector<mitk::DataNode::Pointer> selection;

    if (node != nullptr)
      selection.emplace_back(node);

    mitk::CoreServices::GetNodeSelectionService()->SendSelection(ViewId + "/" + selector, selection);
  }

  mitk::DataNode::Pointer FindNode(const mitk::IRestServerService& server, const nlohmann::json& uid, const std::string& path)
  {
    if (!uid.is_string())
      throw mitk::RestException(mitk::ErrorResponse::InvalidRequest("A node is given by its uid, a string.", path));

    auto node = server.FindNode(uid.get<std::string>());

    if (node.IsNull())
      throw mitk::RestException(mitk::ErrorResponse::NodeNotFound(uid.get<std::string>(), path));

    return node;
  }

  mitk::MultiLabelSegmentation* GetSegmentation(mitk::DataNode* node, const std::string& path)
  {
    auto* segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());

    if (segmentation == nullptr)
      Throw("NOT_A_SEGMENTATION", "Unprocessable Content", "The node \"" + node->GetName() + "\" is no segmentation.", 422, path);

    return segmentation;
  }

  mitk::Label::Pointer GetLabel(mitk::MultiLabelSegmentation* segmentation, const mitk::RestRequest& request)
  {
    mitk::MultiLabelSegmentation::LabelValueType value = 0;

    try
    {
      value = static_cast<mitk::MultiLabelSegmentation::LabelValueType>(std::stoul(request.PathParams.at("value")));
    }
    catch (const std::exception&)
    {
      throw mitk::RestException(mitk::ErrorResponse::InvalidRequest("The label value must be a number.", request.Path));
    }

    auto label = segmentation->GetLabel(value);

    if (label.IsNull())
      Throw("LABEL_NOT_FOUND", "Not Found", "The segmentation has no label " + std::to_string(value) + ".", 404, request.Path);

    return label;
  }

  mitk::MultiLabelSegmentation::GroupIndexType GetGroupIndex(const mitk::MultiLabelSegmentation* segmentation, const mitk::RestRequest& request)
  {
    mitk::MultiLabelSegmentation::GroupIndexType group = 0;

    try
    {
      group = static_cast<mitk::MultiLabelSegmentation::GroupIndexType>(std::stoul(request.PathParams.at("index")));
    }
    catch (const std::exception&)
    {
      throw mitk::RestException(mitk::ErrorResponse::InvalidRequest("The group index must be a number.", request.Path));
    }

    if (!segmentation->ExistGroup(group))
      Throw("GROUP_NOT_FOUND", "Not Found", "The segmentation has no group " + std::to_string(group) + ".", 404, request.Path);

    return group;
  }

  mitk::SegWithPreviewTool* GetPreviewTool(const std::string& path)
  {
    auto* tool = dynamic_cast<mitk::SegWithPreviewTool*>(GetToolManager()->GetActiveTool());

    if (tool == nullptr)
      Throw("NO_PREVIEW_TOOL", "Conflict", "No tool with a result to confirm is active.", 409, path);

    return tool;
  }

  /** Cancels the progress tasks whose names contain \p name, like the downloads and runs of a job. */
  void CancelTasks(const std::string& name)
  {
    auto* progressService = mitk::CoreServices::GetProgressService();

    for (const auto& task : progressService->GetActiveTasks())
    {
      if (task.Cancelable && !task.Finished && task.Name.find(name) != std::string::npos)
        progressService->RequestCancel(task.Id);
    }
  }

  /** Returns an mitk::Exception with the given message, for a callback. */
  std::exception_ptr MakeError(const std::string& message)
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

  /** Ends a job with what an operation threw. */
  void FailJob(mitk::RestJob& job, std::exception_ptr error)
  {
    try
    {
      std::rethrow_exception(error);
    }
    catch (const mitk::RestException& e)
    {
      job.Fail(e.what());
    }
    catch (const mitk::Exception& e)
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

  /**
   * Runs \p then on the GUI thread once the tool neither updates nor runs
   * other work off the GUI thread. Polls rather than listening to the end of
   * the update: a confirmation that follows an update transfers only after
   * the tool has reported that end.
   */
  void WhenToolIdle(const mitk::SegWithPreviewTool::Pointer& tool, std::function<void()> then)
  {
    if (!tool->IsUpdating() && !tool->IsRunningOffGUIThread())
    {
      then();
      return;
    }

    QTimer::singleShot(50, QCoreApplication::instance(), [tool, then = std::move(then)]() mutable {
      WhenToolIdle(tool, std::move(then));
    });
  }

  /** Refuses a request while the tool works, which takes one thing at a time. */
  void ThrowIfBusy(const mitk::SegWithPreviewTool* tool, const std::string& path)
  {
    if (tool->IsUpdating() || tool->IsRunningOffGUIThread())
      Throw("BUSY", "Conflict", std::string(tool->GetName()) + " is still working. Wait for its job to end.", 409, path);
  }

#if MITK_HAS_PYTHON
  mitk::nnInteractiveTool* GetNNInteractive(const std::string& path)
  {
    auto* tool = dynamic_cast<mitk::nnInteractiveTool*>(GetToolManager()->GetActiveTool());

    if (tool == nullptr)
    {
      Throw("TOOL_NOT_ACTIVE", "Conflict",
        "nnInteractive is not active. Activate it with PATCH /api/v1/segmentation/session {\"tool\": \"nnInteractive\"}.", 409, path);
    }

    return tool;
  }

  std::uint64_t CountPreviewVoxels(mitk::SegWithPreviewTool* tool)
  {
    auto* preview = tool->GetPreviewSegmentation();

    if (preview == nullptr)
      return 0;

    const auto timeSteps = GetSelectedTime().GetTimeSteps(preview);

    if (timeSteps.empty())
      return 0;

    std::uint64_t count = 0;

    for (mitk::MultiLabelSegmentation::GroupIndexType group = 0; group < preview->GetNumberOfGroups(); ++group)
    {
      const auto counts = CountVoxels(preview->GetGroupImage(group), timeSteps.front());

      for (std::size_t value = 1; value < counts.size(); ++value)
        count += counts[value];
    }

    return count;
  }

  nlohmann::json DescribeNNInteractive(mitk::nnInteractiveTool* tool)
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
      { "remote", tool->IsRemoteSession() },
      { "prompt_types", prompts },
      { "can_undo", tool->CanUndo() },
      { "auto_zoom", tool->GetAutoZoom() },
      { "auto_refine", tool->GetAutoRefine() },
      { "preview_voxels", running ? CountPreviewVoxels(tool) : 0 }
    };
  }

  mitk::nnInteractive::InteractionType ParseInteractionType(const nlohmann::json& body, const std::string& path)
  {
    const auto type = body.value("type", std::string());

    if (type == "point")
      return mitk::nnInteractive::InteractionType::Point;

    if (type == "box")
      return mitk::nnInteractive::InteractionType::Box;

    if (type == "scribble")
      return mitk::nnInteractive::InteractionType::Scribble;

    if (type == "lasso")
      return mitk::nnInteractive::InteractionType::Lasso;

    throw mitk::RestException(mitk::ErrorResponse::InvalidRequest("\"type\" must be point, box, scribble or lasso.", path));
  }

  mitk::VoxTellTool* GetVoxTell(const std::string& path)
  {
    auto* tool = dynamic_cast<mitk::VoxTellTool*>(GetToolManager()->GetActiveTool());

    if (tool == nullptr)
    {
      Throw("TOOL_NOT_ACTIVE", "Conflict",
        "VoxTell is not active. Activate it with PATCH /api/v1/segmentation/session {\"tool\": \"VoxTell\"}.", 409, path);
    }

    return tool;
  }

  /** Downloads files of the Hugging Face Hub for VoxTell, as its panel does, with a progress notification. */
  void DownloadForVoxTell(const std::string& displayName, const std::vector<mitk::VoxTell::RepoFiles>& files)
  {
    QString output;
    const auto result = QmitkVenvProcess::RunStep(mitk::VoxTell::VENV_NAME, mitk::VoxTell::BuildDownloadStep(displayName, files), &output);

    if (result == QmitkVenvProcess::StepResult::Cancelled)
      mitkThrow() << displayName << " was cancelled.";

    if (result == QmitkVenvProcess::StepResult::Failed)
    {
      MITK_ERROR << "VoxTell: " << displayName << " failed:\n" << output.toStdString();
      mitkThrow() << displayName << " failed. The log tells why.";
    }
  }

  nlohmann::json DescribeVoxTell(mitk::VoxTellTool* tool)
  {
    auto* preview = tool->GetPreviewSegmentation();

    return {
      { "installed", IsInstalled(tool).value_or(false) },
      { "model_loaded", tool->IsModelLoaded() },
      { "prompts", tool->GetPrompts() },
      { "prompts_without_result", tool->GetPromptsWithoutResult() },
      { "error", tool->GetLastErrorMessage().empty() ? nlohmann::json() : nlohmann::json(tool->GetLastErrorMessage()) },
      { "preview_labels", preview != nullptr ? DescribeLabels(preview) : nlohmann::json::array() }
    };
  }

  mitk::TotalSegmentatorTool* GetTotalSegmentator(const std::string& path)
  {
    auto* tool = dynamic_cast<mitk::TotalSegmentatorTool*>(GetToolManager()->GetActiveTool());

    if (tool == nullptr)
    {
      Throw("TOOL_NOT_ACTIVE", "Conflict",
        "TotalSegmentator is not active. Activate it with PATCH /api/v1/segmentation/session {\"tool\": \"TotalSegmentator\"}.", 409, path);
    }

    return tool;
  }

  std::string ToJson(mitk::TotalSegmentatorTool::Speed speed)
  {
    switch (speed)
    {
      case mitk::TotalSegmentatorTool::Speed::Fast:
        return "fast";
      case mitk::TotalSegmentatorTool::Speed::Fastest:
        return "fastest";
      default:
        return "standard";
    }
  }

  mitk::TotalSegmentatorTool::Speed ParseSpeed(const nlohmann::json& json, const std::string& path)
  {
    const auto speed = json.is_string() ? json.get<std::string>() : std::string();

    if (speed == "standard")
      return mitk::TotalSegmentatorTool::Speed::Standard;

    if (speed == "fast")
      return mitk::TotalSegmentatorTool::Speed::Fast;

    if (speed == "fastest")
      return mitk::TotalSegmentatorTool::Speed::Fastest;

    throw mitk::RestException(mitk::ErrorResponse::InvalidRequest("\"speed\" must be standard, fast or fastest.", path));
  }

  mitk::TotalSegmentatorTool* GetInitializedTotalSegmentator(const std::string& path)
  {
    auto* tool = GetTotalSegmentator(path);

    if (!tool->IsInitialized())
    {
      Throw("NOT_INITIALIZED", "Conflict",
        "TotalSegmentator has not started yet. Start it with POST /api/v1/segmentation/totalsegmentator/initialize.", 409, path);
    }

    return tool;
  }

  /** \param withClasses Whether to list the classes of the task, which can be many. */
  nlohmann::json DescribeTotalSegmentatorTask(const mitk::TotalSegmentatorTool::TaskInfo& task, bool withClasses)
  {
    auto speeds = nlohmann::json::array();

    for (const auto& speed : task.Speeds)
    {
      speeds.push_back({
        { "name", ToJson(speed.Value) },
        { "resolution_mm", speed.Resolution > 0 ? nlohmann::json(speed.Resolution) : nlohmann::json() }
      });
    }

    nlohmann::json json = {
      { "name", task.Name },
      { "modality", task.MR ? "MR" : "CT" },
      { "licensed", task.Licensed },
      { "available", !task.Licensed || mitk::TotalSegmentatorTool::HasLicense() },
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

  /** \param withTasks Whether to list the tasks, which only GET does, as there are many. */
  nlohmann::json DescribeTotalSegmentator(mitk::TotalSegmentatorTool* tool, bool withTasks)
  {
    auto* preview = tool->GetPreviewSegmentation();

    nlohmann::json state = {
      { "installed", mitk::TotalSegmentatorTool::IsInstalled() },
      { "initialized", tool->IsInitialized() },
      { "has_license", mitk::TotalSegmentatorTool::HasLicense() },
      { "device", mitk::TotalSegmentatorTool::GetDevice() },
      { "task", tool->GetTask() },
      { "speed", ToJson(tool->GetSpeed()) },
      { "roi_subset", tool->GetRoiSubset() },
      { "running", tool->IsUpdating() },
      { "error", tool->GetLastErrorMessage().empty() ? nlohmann::json() : nlohmann::json(tool->GetLastErrorMessage()) },
      { "preview_labels", preview != nullptr ? DescribeLabels(preview) : nlohmann::json::array() }
    };

    if (withTasks && tool->IsInitialized())
    {
      auto tasks = nlohmann::json::array();

      for (const auto& task : tool->GetTasks())
        tasks.push_back(DescribeTotalSegmentatorTask(task, false));

      state["tasks"] = tasks;
    }

    return state;
  }

  /**
   * Waits until TotalSegmentator has started, or failed to, whoever started
   * it, as the tool tells that only by events. Deletes itself afterwards.
   */
  class TotalSegmentatorStartListener
  {
  public:
    /**
     * Starts TotalSegmentator unless it runs or starts already, then calls
     * \p done on the GUI thread with null, or with why it does not start.
     */
    static void Start(mitk::TotalSegmentatorTool* tool, std::function<void(std::exception_ptr)> done)
    {
      if (tool->IsInitialized())
      {
        done(nullptr);
        return;
      }

      // Listens before the start, which can fail at once.
      new TotalSegmentatorStartListener(tool, std::move(done));
      tool->Initialize(std::make_shared<mitk::ProgressTask>("Starting TotalSegmentator", mitk::ProgressTask::Indeterminate, true));
    }

  private:
    TotalSegmentatorStartListener(mitk::TotalSegmentatorTool* tool, std::function<void(std::exception_ptr)> done)
      : m_Tool(tool),
        m_Done(std::move(done))
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
      this->End(MakeError(message.empty() ? std::string("Starting TotalSegmentator was cancelled.") : message));
    }

    // Not from within the event, which goes on to its other listeners, and
    // after which what follows may run the tool.
    void End(std::exception_ptr error)
    {
      QTimer::singleShot(0, QCoreApplication::instance(), [this, error]() {
        m_Tool->InitializedEvent -= mitk::MessageDelegate<TotalSegmentatorStartListener>(this, &TotalSegmentatorStartListener::OnInitialized);
        m_Tool->InitializationFailedEvent -= mitk::MessageDelegate1<TotalSegmentatorStartListener, std::string>(this, &TotalSegmentatorStartListener::OnInitializationFailed);

        const auto done = std::move(m_Done);
        delete this;

        done(error);
      });
    }

    mitk::TotalSegmentatorTool::Pointer m_Tool;
    std::function<void(std::exception_ptr)> m_Done;
  };
#endif
}

std::vector<mitk::RestRoute> mitk::CreateSegmentationRoutes(IRestServerService* server)
{
  std::vector<RestRoute> routes;

  RestRoute getSession;
  getSession.Method = "GET";
  getSession.Path = "/segmentation/session";
  getSession.Summary = "Get what the Segmentation view works on: image, segmentation, label and tool";
  getSession.Description =
    "Returns {data: {view_open, reference: {uid, name, time_steps} | null, segmentation: {uid, name, time_steps} | "
    "null, active_label: {value, name} | null, tool: name | null, tools: [{name, usable, installed}]}}. The "
    "reference is the image that tools segment; tools write into the active label of the segmentation. "
    "time_steps are the time points that each time step of the data covers, [{min_timepoint_ms, "
    "max_timepoint_ms}], the maximum excluded; data of a single time step has null bounds, as it shows at any "
    "time point. The "
    "tools are known once the Segmentation view has been open. installed is there for tools that need an "
    "installation of their own, like nnInteractive, VoxTell and TotalSegmentator; the user installs them "
    "through the tool's panel in the Segmentation view. Tools download the models they need on their own, "
    "which GET /api/v1/workbench/tasks shows.";
  getSession.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() { return DescribeSession(*server, request.Path); }));
  };
  routes.push_back(getSession);

  RestRoute patchSession;
  patchSession.Method = "PATCH";
  patchSession.Path = "/segmentation/session";
  patchSession.Summary = "Choose the image, segmentation, active label and tool of the Segmentation view";
  patchSession.Description =
    "Body, all optional and applied in this order: {reference: uid of an image, segmentation: uid of a "
    "segmentation of that image or null, active_label: value, tool: name or null}. Opens the Segmentation "
    "view if needed and selects there as the user would. A tool needs an image and a segmentation; "
    "changing them ends the active tool, as in the view, and discards results it did not confirm. Returns "
    "{data: session} as GET does.";
  patchSession.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    // Resolved before going to the GUI thread; see CallInGuiThread().
    DataNode::Pointer reference;
    DataNode::Pointer segmentation;

    if (body.contains("reference"))
      reference = FindNode(*server, body["reference"], request.Path);

    if (body.contains("segmentation") && !body["segmentation"].is_null())
      segmentation = FindNode(*server, body["segmentation"], request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* toolManager = GetToolManager();

      if (reference.IsNotNull())
      {
        SelectInView("referenceNode", reference);

        if (toolManager->GetReferenceData(0) != reference)
        {
          Throw("NOT_ACCEPTED", "Unprocessable Content",
            "The Segmentation view does not take \"" + reference->GetName() + "\" as image to segment.", 422, request.Path);
        }
      }

      if (body.contains("segmentation"))
      {
        SelectInView("workingNode", segmentation);

        if (toolManager->GetWorkingData(0) != segmentation.GetPointer())
        {
          Throw("NOT_ACCEPTED", "Unprocessable Content", segmentation.IsNotNull()
            ? "The Segmentation view does not take \"" + segmentation->GetName() + "\" as segmentation of the image. "
              "A segmentation needs the geometry of the image."
            : std::string("The Segmentation view keeps its segmentation."), 422, request.Path);
        }
      }

      if (body.contains("active_label"))
      {
        auto* working = toolManager->GetWorkingData(0);

        if (working == nullptr)
          Throw("NO_SEGMENTATION", "Conflict", "Choose a segmentation first.", 409, request.Path);

        if (!body["active_label"].is_number_unsigned())
          throw RestException(ErrorResponse::InvalidRequest("\"active_label\" must be the value of a label.", request.Path));

        const auto value = body["active_label"].get<MultiLabelSegmentation::LabelValueType>();
        const auto* segmentation = GetSegmentation(working, request.Path);

        if (!segmentation->ExistLabel(value))
          Throw("LABEL_NOT_FOUND", "Not Found", "The segmentation has no label " + std::to_string(value) + ".", 404, request.Path);

        // Through the tool manager, so that the view selects the label as well.
        toolManager->SetActiveWorkingLabel(value);

        // The view applies the label to the segmentation, or silently does not.
        if (const auto* active = segmentation->GetActiveLabel(); active == nullptr || active->GetValue() != value)
        {
          Throw("NOT_ACCEPTED", "Unprocessable Content", "The Segmentation view did not make label " +
            std::to_string(value) + " active; the active label is " +
            (active != nullptr ? std::to_string(active->GetValue()) : std::string("none")) + ".", 422, request.Path);
        }
      }

      if (body.contains("tool"))
      {
        if (body["tool"].is_null())
        {
          toolManager->ActivateTool(-1);
        }
        else
        {
          const auto name = body["tool"].is_string() ? body["tool"].get<std::string>() : std::string();
          const Tool* tool = nullptr;

          for (const auto& candidate : toolManager->GetTools())
          {
            if (name == candidate->GetName())
              tool = candidate;
          }

          if (tool == nullptr)
            Throw("TOOL_NOT_FOUND", "Not Found", "There is no tool \"" + name + "\". GET /api/v1/segmentation/session lists them.", 404, request.Path);

          if (!toolManager->ActivateTool(toolManager->GetToolID(tool)) || toolManager->GetActiveTool() != tool)
          {
            Throw("TOOL_NOT_ACTIVATED", "Conflict",
              "The tool \"" + name + "\" cannot work on the chosen image and segmentation, or none are chosen.", 409, request.Path);
          }
        }
      }

      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeSession(*server, request.Path);
    }));
  };
  routes.push_back(patchSession);

  RestRoute create;
  create.Method = "POST";
  create.Path = "/segmentation/segmentations";
  create.Summary = "Create a segmentation for an image";
  create.Description =
    "Body: {reference: uid of the image, name (optional), labels (optional): [name or {name, color: "
    "\"#rrggbb\"}]}. Without labels, the segmentation gets one label with a suggested name. The new "
    "segmentation becomes the one the Segmentation view works on, with its first label active. Returns "
    "{data: {uid, name, labels: [label]}} with status 201, labels as GET .../labels gives them.";
  create.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (!body.contains("reference"))
      throw RestException(ErrorResponse::InvalidRequest("\"reference\", the uid of an image, is missing.", request.Path));

    const auto reference = FindNode(*server, body["reference"], request.Path);
    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto node = CallInGuiThread(*server, [&]() {
      const auto* image = dynamic_cast<const Image*>(reference->GetData());

      if (image == nullptr || image->GetDimension() < 2)
        Throw("NOT_AN_IMAGE", "Unprocessable Content", "\"" + reference->GetName() + "\" is no image of two or more dimensions.", 422, request.Path);

      auto newNode = LabelSetImageHelper::CreateNewSegmentationNode(reference, image, body.value("name", std::string()), dataStorage);
      auto* segmentation = GetSegmentation(newNode, request.Path);

      std::vector<Label::Pointer> labels;

      for (const auto& entry : body.value("labels", nlohmann::json::array()))
      {
        auto label = LabelSetImageHelper::CreateNewLabel(segmentation);
        const auto name = entry.is_string() ? entry.get<std::string>() : entry.value("name", std::string());

        if (!name.empty())
          label->SetName(name);

        if (entry.is_object() && entry.contains("color"))
          label->SetColor(ParseColor(entry["color"], request.Path));

        segmentation->AddLabel(label, segmentation->GetActiveLayer(), false);
        labels.push_back(label);
      }

      if (labels.empty())
      {
        auto label = LabelSetImageHelper::CreateNewLabel(segmentation);
        segmentation->AddLabel(label, segmentation->GetActiveLayer(), false);
        labels.push_back(label);
      }

      segmentation->SetActiveLabel(labels.front()->GetValue());
      dataStorage->Add(newNode, reference);

      ShowView(request.Path);
      SelectInView("referenceNode", reference);
      SelectInView("workingNode", newNode);

      return newNode;
    });

    auto json = DescribeNode(node, *server);
    json["labels"] = CallInGuiThread(*server, [&]() { return DescribeLabels(GetSegmentation(node, request.Path)); });

    return RestResponse::FromData(json, 201);
  };
  routes.push_back(create);

  RestRoute labels;
  labels.Method = "GET";
  labels.Path = "/segmentation/segmentations/:uid/labels";
  labels.Summary = "List the labels of a segmentation with their volumes";
  labels.Description =
    "Query parameter: timepoint_ms (optional), a time point in milliseconds, or all for each time step of the "
    "segmentation; the selected time point without it. Returns {data: [{value, name, color, visible, locked, "
    "group, voxels, volume_ml}]}, the voxels and volumes at the time point; with all, each label has time_steps: "
    "[{min_timepoint_ms, max_timepoint_ms, voxels, volume_ml}] instead. voxels is null while a tool writes into "
    "the segmentation, "
    "and when the segmentation has no data at the time point.";
  labels.Handler = [server](const RestRequest& request) {
    const auto node = FindNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      return DescribeLabels(GetSegmentation(node, request.Path), RestTimeSelection::FromRequest(request));
    }));
  };
  routes.push_back(labels);

  RestRoute addLabel;
  addLabel.Method = "POST";
  addLabel.Path = "/segmentation/segmentations/:uid/labels";
  addLabel.Summary = "Add a label to a segmentation";
  addLabel.Description =
    "Body: {name (optional; a suggested name otherwise), color (optional, \"#rrggbb\"), group (optional, "
    "the active group otherwise)}. Undoable. Returns {data: label} with status 201. To segment into it, "
    "make it the active label with PATCH /api/v1/segmentation/session.";
  addLabel.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto node = FindNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node, request.Path);
      const auto group = body.value("group", static_cast<unsigned int>(segmentation->GetActiveLayer()));

      if (group >= segmentation->GetNumberOfGroups())
        Throw("GROUP_NOT_FOUND", "Not Found", "The segmentation has no group " + std::to_string(group) + ".", 404, request.Path);

      auto label = LabelSetImageHelper::CreateNewLabel(segmentation);

      if (body.contains("name"))
        label->SetName(body["name"].get<std::string>());

      if (body.contains("color"))
        label->SetColor(ParseColor(body["color"], request.Path));

      SegGroupModifyUndoRedoHelper undoRedo(segmentation, { group }, false, 0, false, true, true);
      auto* added = segmentation->AddLabel(label, group, false);
      undoRedo.RegisterUndoRedoOperationEvent("Add label \"" + added->GetName() + "\"");

      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeLabel(added, group);
    }), 201);
  };
  routes.push_back(addLabel);

  RestRoute changeLabel;
  changeLabel.Method = "PATCH";
  changeLabel.Path = "/segmentation/segmentations/:uid/labels/:value";
  changeLabel.Summary = "Rename, recolor, show, hide, lock or unlock a label";
  changeLabel.Description =
    "Body, all optional: {name, color: \"#rrggbb\", visible, locked}. Locked labels are kept when tools "
    "write into the segmentation. Renaming and recoloring are undoable. Returns {data: label}.";
  changeLabel.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto node = FindNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node, request.Path);
      auto label = GetLabel(segmentation, request);
      const auto group = segmentation->GetGroupIndexOfLabel(label->GetValue());

      if (body.contains("name") || body.contains("color"))
      {
        const auto name = body.contains("name") ? body["name"].get<std::string>() : label->GetName();
        const auto color = body.contains("color") ? ParseColor(body["color"], request.Path) : label->GetColor();

        SegGroupModifyUndoRedoHelper undoRedo(segmentation, { group }, false, 0, false, true, true);
        segmentation->RenameLabel(label->GetValue(), name, color);
        undoRedo.RegisterUndoRedoOperationEvent("Change label \"" + name + "\"");
      }

      if (body.contains("visible"))
      {
        label->SetVisible(body["visible"].get<bool>());
        segmentation->UpdateLookupTable(label->GetValue());
        segmentation->GetLookupTable()->Modified();
      }

      if (body.contains("locked"))
        label->SetLocked(body["locked"].get<bool>());

      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeLabel(label, group);
    }));
  };
  routes.push_back(changeLabel);

  RestRoute removeLabel;
  removeLabel.Method = "DELETE";
  removeLabel.Path = "/segmentation/segmentations/:uid/labels/:value";
  removeLabel.Summary = "Remove a label and its contents";
  removeLabel.Description = "Undoable. Returns {data: [label]}, the labels that remain, as GET .../labels gives them.";
  removeLabel.Handler = [server](const RestRequest& request) {
    const auto node = FindNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node, request.Path);
      const auto label = GetLabel(segmentation, request);
      const auto name = label->GetName();

      SegGroupModifyUndoRedoHelper undoRedo(segmentation, { segmentation->GetGroupIndexOfLabel(label->GetValue()) }, true);
      segmentation->RemoveLabels({ label->GetValue() });
      undoRedo.RegisterUndoRedoOperationEvent("Remove label \"" + name + "\"");

      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeLabels(segmentation);
    }));
  };
  routes.push_back(removeLabel);

  RestRoute groups;
  groups.Method = "GET";
  groups.Path = "/segmentation/segmentations/:uid/groups";
  groups.Summary = "List the groups of a segmentation with their labels";
  groups.Description =
    "A group holds labels that do not overlap; labels in different groups may overlap. Returns {data: "
    "[group]}, a group being {index, name, display_name, labels: [{value, name, color, visible, locked, "
    "group}]}; name is empty if the group has none, display_name is what the Segmentation view shows.";
  groups.Handler = [server](const RestRequest& request) {
    const auto node = FindNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      return DescribeGroups(GetSegmentation(node, request.Path));
    }));
  };
  routes.push_back(groups);

  RestRoute group;
  group.Method = "GET";
  group.Path = "/segmentation/segmentations/:uid/groups/:index";
  group.Summary = "Get a group of a segmentation with its labels";
  group.Description = "Returns {data: group} as GET .../groups lists it.";
  group.Handler = [server](const RestRequest& request) {
    const auto node = FindNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      auto* segmentation = GetSegmentation(node, request.Path);
      return DescribeGroup(segmentation, GetGroupIndex(segmentation, request));
    }));
  };
  routes.push_back(group);

  RestRoute addGroup;
  addGroup.Method = "POST";
  addGroup.Path = "/segmentation/segmentations/:uid/groups";
  addGroup.Summary = "Add an empty group to a segmentation";
  addGroup.Description =
    "Body, optional: {name}. Undoable. Returns {data: group} with status 201. Add labels to it with POST "
    ".../labels and its index as group. Tools that add labels of their own, like TotalSegmentator and "
    "VoxTell, add them to the group of the active label: to fill this group with them, add a label to it and "
    "make that label active first.";
  addGroup.Handler = [server](const RestRequest& request) {
    const auto body = request.Body.empty() ? nlohmann::json::object() : request.GetJsonBody();
    const auto node = FindNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node, request.Path);
      const auto added = segmentation->AddGroup();

      if (body.contains("name"))
        segmentation->SetGroupName(added, body["name"].get<std::string>());

      SegGroupInsertUndoRedoHelper undoRedo(segmentation, { added }, false, true);
      undoRedo.RegisterUndoRedoOperationEvent("Insert new group \"" + LabelSetImageHelper::CreateDisplayGroupName(segmentation, added) + "\"");

      node->Modified();
      return DescribeGroup(segmentation, added);
    }), 201);
  };
  routes.push_back(addGroup);

  RestRoute changeGroup;
  changeGroup.Method = "PATCH";
  changeGroup.Path = "/segmentation/segmentations/:uid/groups/:index";
  changeGroup.Summary = "Rename a group of a segmentation";
  changeGroup.Description = "Body: {name}; an empty name gives the group its default display name back. Undoable. Returns {data: group}.";
  changeGroup.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (!body.contains("name") || !body["name"].is_string())
      throw RestException(ErrorResponse::InvalidRequest("\"name\", the new name of the group, is missing.", request.Path));

    const auto node = FindNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node, request.Path);
      const auto index = GetGroupIndex(segmentation, request);

      SegGroupModifyUndoRedoHelper undoRedo(segmentation, { index }, true, 0, true, true);
      segmentation->SetGroupName(index, body["name"].get<std::string>());
      undoRedo.RegisterUndoRedoOperationEvent("Rename group #" + std::to_string(index));

      node->Modified();
      return DescribeGroup(segmentation, index);
    }));
  };
  routes.push_back(changeGroup);

  RestRoute removeGroup;
  removeGroup.Method = "DELETE";
  removeGroup.Path = "/segmentation/segmentations/:uid/groups/:index";
  removeGroup.Summary = "Remove a group with its labels";
  removeGroup.Description =
    "Removes the group and every label in it, like Delete group in the Segmentation view. A segmentation "
    "keeps at least one group, so its last group stays (409). The groups after it move down by one index. "
    "Undoable. Returns {data: [group]}, the groups that remain, as GET .../groups lists them.";
  removeGroup.Handler = [server](const RestRequest& request) {
    const auto node = FindNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node, request.Path);
      const auto index = GetGroupIndex(segmentation, request);

      // MultiLabelSegmentation::RemoveGroup() would also remove the last one.
      if (segmentation->GetNumberOfGroups() < 2)
        Throw("LAST_GROUP", "Conflict", "A segmentation keeps at least one group, and this is its last.", 409, request.Path);

      const auto name = LabelSetImageHelper::CreateDisplayGroupName(segmentation, index);

      SegGroupRemoveUndoRedoHelper undoRedo(segmentation, { index });
      segmentation->RemoveGroup(index);
      undoRedo.RegisterUndoRedoOperationEvent("Remove group \"" + name + "\"");

      node->Modified();
      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeGroups(segmentation);
    }));
  };
  routes.push_back(removeGroup);

  RestRoute confirm;
  confirm.Method = "POST";
  confirm.Path = "/segmentation/confirm";
  confirm.Summary = "Confirm the result of the active tool into the active label";
  confirm.Description =
    "Writes the preview of the active tool into the segmentation, like its Confirm button. Body, all "
    "optional: {merge: false, ignore_locks: false, all_time_steps: false}, the options under the button. "
    "Undoable. Returns {data: {segmentation: {uid, name}, labels: [label]}} as GET .../labels gives them; "
    "check there that the label is filled. If the tool has to compute the result first, for all_time_steps "
    "with a tool that previews only the time step on display, or for an active label that changed since the "
    "result was computed, which takes as long as that many runs, the confirm runs as a job: query parameter "
    "wait (seconds) waits for it, and the job's result is the answer.";
  confirm.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const bool allTimeSteps = body.value("all_time_steps", false);

    struct Outcome
    {
      std::optional<nlohmann::json> Data;
      std::shared_ptr<RestJob> Job;
    };

    const auto outcome = CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      SegWithPreviewTool::Pointer tool = GetPreviewTool(request.Path);
      ThrowIfBusy(tool, request.Path);

      tool->SetMergeStyle(body.value("merge", false) ? MultiLabelSegmentation::MergeStyle::Merge : MultiLabelSegmentation::MergeStyle::Replace);
      tool->SetOverwriteStyle(body.value("ignore_locks", false) ? MultiLabelSegmentation::OverwriteStyle::IgnoreLocks : MultiLabelSegmentation::OverwriteStyle::RegardLocks);
      tool->SetCreateAllTimeSteps(allTimeSteps);

      // Held, as the tool may deactivate itself after confirming.
      const DataNode::Pointer target = tool->GetTargetSegmentationNode();
      const auto describe = [server, target, path = request.Path]() {
        RenderingManager::GetInstance()->RequestUpdateAll();

        return nlohmann::json{
          { "segmentation", DescribeNode(target, *server) },
          { "labels", target != nullptr ? DescribeLabels(GetSegmentation(target, path)) : nlohmann::json::array() }
        };
      };

      tool->ConfirmSegmentation();

      if (!tool->IsUpdating())
        return Outcome{ describe(), nullptr };

      // The transfer follows the update.
      auto job = server->CreateJob("Confirm the result");
      job->SetCancelHandler([name = std::string(tool->GetName())]() { CancelTasks(name); });

      WhenToolIdle(tool, [job, describe]() {
        try
        {
          job->Succeed(describe());
        }
        catch (...)
        {
          FailJob(*job, std::current_exception());
        }
      });

      return Outcome{ std::nullopt, job };
    });

    if (outcome.Data.has_value())
      return RestResponse::FromData(*outcome.Data);

    return RestJob::Respond(request, outcome.Job);
  };
  routes.push_back(confirm);

#if MITK_HAS_PYTHON
  RestRoute nnInteractive;
  nnInteractive.Method = "GET";
  nnInteractive.Path = "/segmentation/nninteractive";
  nnInteractive.Summary = "Get the state of nnInteractive, the interactive segmentation tool";
  nnInteractive.Description =
    "Needs nnInteractive as active tool. Returns {data: {session_running, remote, prompt_types, can_undo, "
    "auto_zoom, auto_refine, preview_voxels}}; preview_voxels is the size of the unconfirmed result.";
  nnInteractive.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() { return DescribeNNInteractive(GetNNInteractive(request.Path)); }));
  };
  routes.push_back(nnInteractive);

  RestRoute nnInteractiveOptions;
  nnInteractiveOptions.Method = "PATCH";
  nnInteractiveOptions.Path = "/segmentation/nninteractive";
  nnInteractiveOptions.Summary = "Change the options of nnInteractive";
  nnInteractiveOptions.Description =
    "As the checkboxes of its panel. Body, all optional: {auto_zoom: true|false, auto_refine: true|false}. "
    "auto_zoom widens the region the model looks at when a result reaches its edge, for large structures, "
    "at the cost of speed; it needs a GPU. auto_refine runs the model on the content of a label when it "
    "becomes the target of the session, instead of taking the content as it is until the next prompt. "
    "Returns {data: state} as GET does.";
  nnInteractiveOptions.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    for (const auto* key : { "auto_zoom", "auto_refine" })
    {
      if (body.contains(key) && !body[key].is_boolean())
        throw RestException(ErrorResponse::InvalidRequest(std::string("\"") + key + "\" must be true or false.", request.Path));
    }

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetNNInteractive(request.Path);

      if (body.contains("auto_zoom"))
      {
        const auto autoZoom = body["auto_zoom"].get<bool>();

        if (autoZoom && tool->GetBackend() == nnInteractive::Backend::CPU)
          Throw("NOT_AVAILABLE", "Conflict", "Auto-zoom is not available with the CPU backend.", 409, request.Path);

        tool->SetAutoZoom(autoZoom);
      }

      if (body.contains("auto_refine"))
        tool->SetAutoRefine(body["auto_refine"].get<bool>());

      return DescribeNNInteractive(tool);
    }));
  };
  routes.push_back(nnInteractiveOptions);

  // Only one start at a time: a start loads the model off the GUI thread,
  // during which a second request would get the job of the first.
  struct Start
  {
    std::mutex Mutex;
    std::shared_ptr<RestJob> Job;
    std::optional<ProgressTaskId> TaskId;
  };

  auto start = std::make_shared<Start>();

  RestRoute startSession;
  startSession.Method = "POST";
  startSession.Path = "/segmentation/nninteractive/session";
  startSession.Summary = "Start an nnInteractive session, as its Initialize button does";
  startSession.Description =
    "Needs nnInteractive as active tool, installed once by the user through its Initialize button. Loads "
    "the model, which can take a while the first time, as a job: query parameter wait (seconds) waits for "
    "it. The job's result is the state as GET /api/v1/segmentation/nninteractive gives it. The session works "
    "on the image and time step chosen now; changing them ends it.";
  startSession.Handler = [server, start](const RestRequest& request) {
    CallInGuiThread(*server, [&]() {
      ShowView(request.Path);

      if (!IsInstalled(GetNNInteractive(request.Path)).value_or(false))
      {
        Throw("TOOL_NOT_INSTALLED", "Conflict",
          "nnInteractive is not installed. Ask the user to install it with the Initialize button of the nnInteractive panel.", 409, request.Path);
      }
    });

    std::shared_ptr<RestJob> job;

    {
      std::lock_guard<std::mutex> lock(start->Mutex);

      if (start->Job != nullptr && !start->Job->HasEnded())
      {
        job = start->Job;
      }
      else
      {
        start->Job = server->CreateJob("Start an nnInteractive session");
        start->TaskId.reset();
      }
    }

    if (job != nullptr)
      return RestJob::Respond(request, job);

    job = start->Job;

    job->SetCancelHandler([start]() {
      std::lock_guard<std::mutex> lock(start->Mutex);

      if (start->TaskId.has_value())
        CoreServices::GetProgressService()->RequestCancel(start->TaskId.value());
    });

    server->Post([job, start, path = request.Path]() {
      try
      {
        // Held, as the tool can be deactivated while the start runs.
        nnInteractiveTool::Pointer tool(GetNNInteractive(path));

        auto task = std::make_shared<ProgressTask>("Initializing nnInteractive", ProgressTask::Indeterminate, true);

        {
          std::lock_guard<std::mutex> lock(start->Mutex);
          start->TaskId = task->GetId();
        }

        tool->StartSession(task, [job, task, tool](std::exception_ptr error) {
          if (error != nullptr)
          {
            FailJob(*job, error);
            return;
          }

          task->Finish();
          job->Succeed(DescribeNNInteractive(tool));
        });
      }
      catch (...)
      {
        FailJob(*job, std::current_exception());
      }
    });

    return RestJob::Respond(request, job);
  };
  routes.push_back(startSession);

  RestRoute endSession;
  endSession.Method = "DELETE";
  endSession.Path = "/segmentation/nninteractive/session";
  endSession.Summary = "End the nnInteractive session, discarding unconfirmed results";
  endSession.Description = "As unchecking its Initialize button. Returns {data: state} as GET /api/v1/segmentation/nninteractive does.";
  endSession.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetNNInteractive(request.Path);
      tool->AbortSession();
      return DescribeNNInteractive(tool);
    }));
  };
  routes.push_back(endSession);

  RestRoute prompt;
  prompt.Method = "POST";
  prompt.Path = "/segmentation/nninteractive/prompts";
  prompt.Summary = "Prompt nnInteractive with a point, box, scribble or lasso";
  prompt.Description =
    "Needs a running session. Body: {type: point|box|scribble|lasso, positive: true (the structure, "
    "default) or false (what does not belong to it), and the points: either pixels: [[x, y], ...] of a "
    "screenshot of a 2D window with window and image_size, as GET /api/v1/rendering/windows/{id}/screenshot "
    "describes it, or points: [[x, y, z], ...] in millimeters, all in one slice of the image}. A point "
    "needs one point; a box two opposite corners; a scribble a line of points, painted with the brush of "
    "the panel, three pixels wide; a lasso a closed contour of at least three points, whose inside is the "
    "prompt. The prompt is drawn as if the user had drawn it with the mouse in that window, and the 2D "
    "windows show it. Prompts add up and refine the unconfirmed result, which POST "
    "/api/v1/segmentation/confirm writes into the active label. The model computes the prompt as a job: "
    "query parameter wait (seconds) waits for it, and the job's result is the state as GET "
    "/api/v1/segmentation/nninteractive gives it. One prompt at a time: a prompt while one computes "
    "gets 409.";
  prompt.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto type = ParseInteractionType(body, request.Path);
    const auto promptType = body.value("positive", true) ? nnInteractive::PromptType::Positive : nnInteractive::PromptType::Negative;

    const auto job = CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      nnInteractiveTool::Pointer tool(GetNNInteractive(request.Path));
      ThrowIfBusy(tool, request.Path);

      const auto points = GetRestWorldPoints(body, request.Path);

      // The prompt is drawn in the plane of the window that the pixels were
      // drawn in, as with the mouse.
      const PlaneGeometry* plane = body.contains("pixels")
        ? GetRestRenderer(body["window"].get<std::string>(), request.Path)->GetCurrentWorldPlaneGeometry()
        : nullptr;

      auto job = server->CreateJob("Prompt nnInteractive");
      job->SetCancelHandler([]() { CancelTasks("nnInteractive"); });

      try
      {
        tool->AddPrompt(type, points, promptType, plane, [job, tool](std::exception_ptr error) {
          if (error != nullptr)
          {
            FailJob(*job, error);
            return;
          }

          job->Succeed(DescribeNNInteractive(tool));
        });
      }
      catch (const Exception& e)
      {
        job->Fail(e.GetDescription());
        Throw("PROMPT_FAILED", "Unprocessable Content", e.GetDescription(), 422, request.Path);
      }

      return job;
    });

    return RestJob::Respond(request, job);
  };
  routes.push_back(prompt);

  RestRoute undoPrompt;
  undoPrompt.Method = "POST";
  undoPrompt.Path = "/segmentation/nninteractive/undo";
  undoPrompt.Summary = "Undo the last nnInteractive prompt";
  undoPrompt.Description =
    "nnInteractive undoes one prompt only: after an undo, the next prompt is undoable again. Returns {data: "
    "state}, or 409 if there is nothing to undo.";
  undoPrompt.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetNNInteractive(request.Path);

      if (!tool->CanUndo())
        Throw("NOTHING_TO_UNDO", "Conflict", "There is no prompt to undo.", 409, request.Path);

      tool->UndoLastInteraction();
      return DescribeNNInteractive(tool);
    }));
  };
  routes.push_back(undoPrompt);

  RestRoute reset;
  reset.Method = "POST";
  reset.Path = "/segmentation/nninteractive/reset";
  reset.Summary = "Discard all nnInteractive prompts and the unconfirmed result";
  reset.Description = "The session keeps running. Returns {data: state}.";
  reset.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetNNInteractive(request.Path);
      tool->ResetInteractions();
      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeNNInteractive(tool);
    }));
  };
  routes.push_back(reset);

  RestRoute voxTell;
  voxTell.Method = "GET";
  voxTell.Path = "/segmentation/voxtell";
  voxTell.Summary = "Get the state of VoxTell, which segments structures named in text";
  voxTell.Description =
    "Needs VoxTell as active tool. Returns {data: {installed, model_loaded, prompts, prompts_without_result, "
    "error, preview_labels: [label]}}, preview_labels being the unconfirmed result, one label per prompt.";
  voxTell.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() { return DescribeVoxTell(GetVoxTell(request.Path)); }));
  };
  routes.push_back(voxTell);

  RestRoute voxTellKnownPrompts;
  voxTellKnownPrompts.Method = "GET";
  voxTellKnownPrompts.Path = "/segmentation/voxtell/known-prompts";
  voxTellKnownPrompts.Summary = "Search the prompts VoxTell knows, which need no text model";
  voxTellKnownPrompts.Description =
    "Needs VoxTell as active tool with its model loaded. VoxTell knows about 14,000 anatomical terms, in "
    "lower case, and the prompts it segmented in this session; any other prompt needs its text model of "
    "about 8 GB. Query parameters: query (optional), words that a prompt must all contain, case does not "
    "matter; limit (optional, default 50, at most 500). Returns {data: {count, prompts}}, count being the "
    "number of matches and prompts the first of them: an exact match first, then prompts that start with the "
    "query, then the others, each alphabetically.";
  voxTellKnownPrompts.Handler = [server](const RestRequest& request) {
    std::size_t limit = 50;

    if (const auto value = request.GetQueryParam("limit"); value.has_value())
    {
      try
      {
        limit = std::clamp<std::size_t>(std::stoul(*value), 1, 500);
      }
      catch (const std::exception&)
      {
        throw RestException(ErrorResponse::InvalidRequest("limit must be a number.", request.Path));
      }
    }

    const auto toLower = [](std::string text) {
      std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
      return text;
    };

    const auto query = toLower(request.GetQueryParam("query").value_or(""));
    std::vector<std::string> words;
    std::istringstream stream(query);

    for (std::string word; stream >> word;)
      words.push_back(word);

    const auto known = CallInGuiThread(*server, [&]() {
      auto* tool = GetVoxTell(request.Path);

      if (!tool->IsModelLoaded())
      {
        Throw("MODEL_NOT_LOADED", "Conflict",
          "VoxTell has no model loaded. Load it with POST /api/v1/segmentation/voxtell/model.", 409, request.Path);
      }

      return tool->GetKnownPrompts();
    });

    std::vector<std::string> exact, starting, containing;

    for (const auto& prompt : known)
    {
      const auto lower = toLower(prompt);

      const bool matches = std::all_of(words.begin(), words.end(), [&lower](const std::string& word) {
        return lower.find(word) != std::string::npos;
      });

      if (!matches)
        continue;

      if (lower == query)
        exact.push_back(prompt);
      else if (!query.empty() && 0 == lower.compare(0, query.size(), query))
        starting.push_back(prompt);
      else
        containing.push_back(prompt);
    }

    auto prompts = nlohmann::json::array();

    for (const auto* matches : { &exact, &starting, &containing })
    {
      for (const auto& prompt : *matches)
      {
        if (prompts.size() < limit)
          prompts.push_back(prompt);
      }
    }

    return RestResponse::FromData({ { "count", exact.size() + starting.size() + containing.size() }, { "prompts", prompts } });
  };
  routes.push_back(voxTellKnownPrompts);

  RestRoute voxTellModel;
  voxTellModel.Method = "POST";
  voxTellModel.Path = "/segmentation/voxtell/model";
  voxTellModel.Summary = "Load the model of VoxTell, as the Initialize button of its panel does";
  voxTellModel.Description =
    "Needs VoxTell as active tool, installed by the user through its panel. Downloads the files of the model "
    "first if they are missing, which the progress notifications and GET /api/v1/workbench/tasks show, then "
    "loads it. Runs as a job: query parameter wait (seconds) waits for it. The job's result is the state as "
    "GET /api/v1/segmentation/voxtell gives it.";
  voxTellModel.Handler = [server](const RestRequest& request) {
    const auto loaded = CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetVoxTell(request.Path);

      if (!IsInstalled(tool).value_or(false))
      {
        Throw("TOOL_NOT_INSTALLED", "Conflict",
          "VoxTell is not installed. Ask the user to install it with the Initialize button of the VoxTell panel.", 409, request.Path);
      }

      return tool->IsModelLoaded() ? std::optional<nlohmann::json>(DescribeVoxTell(tool)) : std::nullopt;
    });

    if (loaded.has_value())
      return RestResponse::FromData(*loaded);

    auto job = server->CreateJob("Load the VoxTell model");
    job->SetCancelHandler([]() { CancelTasks("VoxTell"); });

    server->Post([job, path = request.Path]() {
      VoxTellTool::Pointer tool;

      // A worker that the start left behind holds the imports of VoxTell.
      const auto fail = [job](const VoxTellTool::Pointer& tool, std::exception_ptr error) {
        if (tool.IsNotNull() && !tool->IsModelLoaded() && !tool->IsRunningOffGUIThread())
          tool->UnloadModel();

        FailJob(*job, error);
      };

      try
      {
        // Held, as the tool can be deactivated while the worker loads.
        tool = GetVoxTell(path);

        auto filesTask = std::make_shared<ProgressTask>("Preparing VoxTell", ProgressTask::Indeterminate, true);

        tool->GetModelFiles(filesTask, [job, tool, fail](std::exception_ptr error, const std::vector<VoxTell::RepoFiles>& files) {
          try
          {
            if (error != nullptr)
              std::rethrow_exception(error);

            if (!files.empty())
              DownloadForVoxTell("Downloading VoxTell model", files);

            auto loadTask = std::make_shared<ProgressTask>("Loading VoxTell", ProgressTask::Indeterminate, true);

            tool->LoadModel(loadTask, [job, tool, fail, loadTask](std::exception_ptr loadError) {
              if (loadError != nullptr)
              {
                fail(tool, loadError);
                return;
              }

              loadTask->Finish();
              job->Succeed(DescribeVoxTell(tool));
            });
          }
          catch (...)
          {
            fail(tool, std::current_exception());
          }
        });
      }
      catch (...)
      {
        fail(tool, std::current_exception());
      }
    });

    return RestJob::Respond(request, job);
  };
  routes.push_back(voxTellModel);

  RestRoute voxTellPrompts;
  voxTellPrompts.Method = "POST";
  voxTellPrompts.Path = "/segmentation/voxtell/prompts";
  voxTellPrompts.Summary = "Segment the structures that text prompts name, with VoxTell";
  voxTellPrompts.Description =
    "Needs VoxTell as active tool with its model loaded (POST /api/v1/segmentation/voxtell/model). Body: "
    "{prompts: [\"spleen\", ...], create_groups: false (true puts overlapping structures into groups of their "
    "own)}. The VoxTell panel shows the prompts. Prompts VoxTell does not know need its text model, which is "
    "downloaded first if it is missing (about 8 GB) and loaded, which takes a while. Runs as a job: query "
    "parameter wait (seconds) waits for it. The job's result is the state as GET "
    "/api/v1/segmentation/voxtell gives it; POST /api/v1/segmentation/confirm then adds one label per "
    "prompt to the segmentation.";
  voxTellPrompts.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    std::vector<std::string> prompts;

    for (const auto& prompt : body.value("prompts", nlohmann::json::array()))
    {
      if (prompt.is_string() && !prompt.get<std::string>().empty())
        prompts.push_back(prompt.get<std::string>());
    }

    if (prompts.empty())
      throw RestException(ErrorResponse::InvalidRequest("\"prompts\" must list at least one structure.", request.Path));

    const bool createGroups = body.value("create_groups", false);

    CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetVoxTell(request.Path);

      if (!tool->IsModelLoaded())
      {
        Throw("MODEL_NOT_LOADED", "Conflict",
          "VoxTell has no model loaded. Load it with POST /api/v1/segmentation/voxtell/model.", 409, request.Path);
      }

      ThrowIfBusy(tool, request.Path);
    });

    auto job = server->CreateJob("Segment with VoxTell");
    job->SetCancelHandler([]() { CancelTasks("VoxTell"); });

    server->Post([job, prompts, createGroups, path = request.Path]() {
      try
      {
        // Held, as the tool can be deactivated while the model computes.
        VoxTellTool::Pointer tool(GetVoxTell(path));
        const auto unknownPrompts = tool->GetPromptsWithoutPrecomputedEmbedding(prompts);

        // The run, once the prompts are known. The update ends after this
        // has returned.
        const auto run = [job, tool, prompts, createGroups]() {
          if (tool->IsUpdating() || tool->IsRunningOffGUIThread())
          {
            job->Fail("VoxTell is still working.");
            return;
          }

          tool->SetPrompts(prompts);
          tool->SetCreateGroupsAsNeeded(createGroups);
          tool->UpdatePreview();

          WhenToolIdle(tool, [job, tool]() {
            if (!tool->GetLastErrorMessage().empty())
            {
              job->Fail(tool->GetLastErrorMessage());
              return;
            }

            if (tool->WasLastUpdateCancelled())
            {
              job->Fail("VoxTell was cancelled.");
              return;
            }

            job->Succeed(DescribeVoxTell(tool));
          });
        };

        if (unknownPrompts.empty())
        {
          run();
          return;
        }

        if (!tool->IsTextModelLoaded() && !tool->IsTextModelCached())
        {
          if (const auto files = tool->GetTextModelFiles(); !files.empty())
            DownloadForVoxTell("Downloading VoxTell text model", files);
        }

        auto task = std::make_shared<ProgressTask>("Processing prompts with the VoxTell text model", ProgressTask::Indeterminate, true);

        tool->EmbedPrompts(unknownPrompts, task, [job, run](std::exception_ptr error) {
          if (error != nullptr)
          {
            FailJob(*job, error);
            return;
          }

          run();
        });
      }
      catch (...)
      {
        FailJob(*job, std::current_exception());
      }
    });

    return RestJob::Respond(request, job);
  };
  routes.push_back(voxTellPrompts);

  RestRoute totalSegmentator;
  totalSegmentator.Method = "GET";
  totalSegmentator.Path = "/segmentation/totalsegmentator";
  totalSegmentator.Summary = "Get the state of TotalSegmentator, which segments the structures of a body region at once";
  totalSegmentator.Description =
    "Needs TotalSegmentator as active tool. Returns {data: {installed, initialized, has_license, device, task, speed, "
    "roi_subset: [class], running, error, preview_labels: [label], tasks: [{name, modality: CT | MR, licensed, "
    "available, speeds: [{name: standard | fast | fastest, resolution_mm}], roi_subset, class_count}]}}. task, speed "
    "and roi_subset are what the next run segments; an empty roi_subset means all classes of the task. available is "
    "false for licensed tasks without a license. The tasks are there once TotalSegmentator has started (initialized). "
    "preview_labels are the unconfirmed result of the last run. Device and license are settings of the user.";
  totalSegmentator.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() { return DescribeTotalSegmentator(GetTotalSegmentator(request.Path), true); }));
  };
  routes.push_back(totalSegmentator);

  RestRoute totalSegmentatorStart;
  totalSegmentatorStart.Method = "POST";
  totalSegmentatorStart.Path = "/segmentation/totalsegmentator/initialize";
  totalSegmentatorStart.Summary = "Start TotalSegmentator, which learns its tasks";
  totalSegmentatorStart.Description =
    "Needs TotalSegmentator as active tool, installed by the user through its panel. Its panel starts it as well when "
    "it opens. Runs as a job if it has not started yet: query parameter wait (seconds) waits for it. The result is "
    "the state as GET /api/v1/segmentation/totalsegmentator gives it, without tasks.";
  totalSegmentatorStart.Handler = [server](const RestRequest& request) {
    const auto initialized = CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetTotalSegmentator(request.Path);

      if (!TotalSegmentatorTool::IsInstalled())
      {
        Throw("TOOL_NOT_INSTALLED", "Conflict",
          "TotalSegmentator is not installed. Ask the user to install it with the button of the TotalSegmentator panel.", 409, request.Path);
      }

      return tool->IsInitialized() ? std::optional<nlohmann::json>(DescribeTotalSegmentator(tool, false)) : std::nullopt;
    });

    if (initialized.has_value())
      return RestResponse::FromData(*initialized);

    auto job = server->CreateJob("Start TotalSegmentator");
    job->SetCancelHandler([]() { CancelTasks("TotalSegmentator"); });

    server->Post([job, path = request.Path]() {
      try
      {
        // Held, as the tool can be deactivated while it starts.
        TotalSegmentatorTool::Pointer tool(GetTotalSegmentator(path));

        TotalSegmentatorStartListener::Start(tool, [job, tool](std::exception_ptr error) {
          if (error != nullptr)
          {
            FailJob(*job, error);
            return;
          }

          job->Succeed(DescribeTotalSegmentator(tool, false));
        });
      }
      catch (...)
      {
        FailJob(*job, std::current_exception());
      }
    });

    return RestJob::Respond(request, job);
  };
  routes.push_back(totalSegmentatorStart);

  RestRoute totalSegmentatorTaskDetails;
  totalSegmentatorTaskDetails.Method = "GET";
  totalSegmentatorTaskDetails.Path = "/segmentation/totalsegmentator/tasks/:name";
  totalSegmentatorTaskDetails.Summary = "Get a task of TotalSegmentator with the classes it segments";
  totalSegmentatorTaskDetails.Description =
    "Needs TotalSegmentator started. Returns {data: task} as GET /api/v1/segmentation/totalsegmentator lists it, plus "
    "classes: [name], the exact names a roi_subset takes and the labels of a result get.";
  totalSegmentatorTaskDetails.Handler = [server](const RestRequest& request) {
    const auto name = request.PathParams.at("name");

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      const auto* task = GetInitializedTotalSegmentator(request.Path)->FindTask(name);

      if (task == nullptr)
        Throw("TASK_NOT_FOUND", "Not Found", "TotalSegmentator has no task \"" + name + "\".", 404, request.Path);

      return DescribeTotalSegmentatorTask(*task, true);
    }));
  };
  routes.push_back(totalSegmentatorTaskDetails);

  RestRoute totalSegmentatorClasses;
  totalSegmentatorClasses.Method = "GET";
  totalSegmentatorClasses.Path = "/segmentation/totalsegmentator/classes";
  totalSegmentatorClasses.Summary = "Find the TotalSegmentator tasks that segment a structure";
  totalSegmentatorClasses.Description =
    "Needs TotalSegmentator started. Query parameter name, once or repeated, is an exact class name like liver or "
    "kidney_left; returns {data: {name: [task]}}. Without name, returns {data: {CT: [class], MR: [class]}}, all "
    "classes of the tasks for each modality.";
  totalSegmentatorClasses.Handler = [server](const RestRequest& request) {
    std::vector<std::string> names;

    for (auto [it, end] = request.QueryParams.equal_range("name"); it != end; ++it)
      names.push_back(it->second);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      const auto* tool = GetInitializedTotalSegmentator(request.Path);
      nlohmann::json result = nlohmann::json::object();

      if (names.empty())
      {
        std::map<std::string, std::set<std::string>> classes;

        for (const auto& task : tool->GetTasks())
        {
          for (const auto& [value, name] : task.Classes)
            classes[task.MR ? "MR" : "CT"].insert(name);
        }

        for (const auto& [modality, modalityClasses] : classes)
          result[modality] = modalityClasses;

        return result;
      }

      for (const auto& name : names)
      {
        auto tasks = nlohmann::json::array();

        for (const auto& task : tool->GetTasks())
        {
          for (const auto& [value, className] : task.Classes)
          {
            if (className == name)
            {
              tasks.push_back(task.Name);
              break;
            }
          }
        }

        result[name] = tasks;
      }

      return result;
    }));
  };
  routes.push_back(totalSegmentatorClasses);

  RestRoute totalSegmentatorOptions;
  totalSegmentatorOptions.Method = "PATCH";
  totalSegmentatorOptions.Path = "/segmentation/totalsegmentator";
  totalSegmentatorOptions.Summary = "Choose the task, speed and structures of TotalSegmentator";
  totalSegmentatorOptions.Description =
    "As the task box and the structure list of its panel. Needs TotalSegmentator started. Body, all optional: {task: "
    "a name of GET's tasks, speed: standard | fast | fastest, roi_subset: [class]}. fast and fastest run coarser "
    "models (resolution_mm of the task's speeds), quicker; speed is standard unless given with a task. A new task "
    "segments all of its classes, unless roi_subset, the exact class names of GET .../tasks/{name}, chooses some: "
    "only tasks with roi_subset true take one, and an empty list chooses all. Returns {data: state} as GET does, "
    "without tasks.";
  totalSegmentatorOptions.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (body.contains("task") && !body["task"].is_string())
      throw RestException(ErrorResponse::InvalidRequest("\"task\" must be the name of a task.", request.Path));

    if (body.contains("roi_subset") && (!body["roi_subset"].is_array() || std::any_of(body["roi_subset"].begin(), body["roi_subset"].end(), [](const auto& item) { return !item.is_string(); })))
      throw RestException(ErrorResponse::InvalidRequest("\"roi_subset\" must be a list of class names.", request.Path));

    const auto speed = body.contains("speed")
      ? std::optional<TotalSegmentatorTool::Speed>(ParseSpeed(body["speed"], request.Path))
      : std::nullopt;

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetInitializedTotalSegmentator(request.Path);

      if (tool->IsUpdating())
        Throw("BUSY", "Conflict", "TotalSegmentator is running. Wait until the run has ended.", 409, request.Path);

      try
      {
        if (body.contains("task") || speed.has_value())
        {
          const auto task = body.contains("task") ? body["task"].get<std::string>() : tool->GetTask();
          tool->SetTask(task, speed.value_or(body.contains("task") ? TotalSegmentatorTool::Speed::Standard : tool->GetSpeed()));
        }

        if (body.contains("roi_subset"))
          tool->SetRoiSubset(body["roi_subset"].get<std::vector<std::string>>());
      }
      catch (const Exception& e)
      {
        throw RestException(ErrorResponse::InvalidRequest(e.GetDescription(), request.Path));
      }

      return DescribeTotalSegmentator(tool, false);
    }));
  };
  routes.push_back(totalSegmentatorOptions);

  RestRoute totalSegmentatorRun;
  totalSegmentatorRun.Method = "POST";
  totalSegmentatorRun.Path = "/segmentation/totalsegmentator/run";
  totalSegmentatorRun.Summary = "Run TotalSegmentator with the chosen task, as the Run button of its panel does";
  totalSegmentatorRun.Description =
    "Needs TotalSegmentator as active tool, installed by the user through its panel; starts it if needed. Segments "
    "the reference image with the task, speed and roi_subset chosen with PATCH /api/v1/segmentation/totalsegmentator. "
    "Runs as a job, which takes seconds to minutes; the first run of a task downloads its model weights first, which "
    "the progress notifications and GET /api/v1/workbench/tasks show. Query parameter wait (seconds) waits for it. "
    "The job's result is the state as GET /api/v1/segmentation/totalsegmentator gives it, without tasks; POST "
    "/api/v1/segmentation/confirm then adds the labels of preview_labels to the segmentation. preview_labels has a "
    "label for each class found in the image, and is empty if none was.";
  totalSegmentatorRun.Handler = [server](const RestRequest& request) {
    CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetTotalSegmentator(request.Path);

      if (!TotalSegmentatorTool::IsInstalled())
      {
        Throw("TOOL_NOT_INSTALLED", "Conflict",
          "TotalSegmentator is not installed. Ask the user to install it with the button of the TotalSegmentator panel.", 409, request.Path);
      }

      if (const auto* task = tool->FindTask(tool->GetTask()); task != nullptr && task->Licensed && !TotalSegmentatorTool::HasLicense())
      {
        Throw("LICENSE_REQUIRED", "Conflict",
          "The task \"" + tool->GetTask() + "\" requires a TotalSegmentator license, which the user sets in its settings. "
          "Choose a task that is available, or ask the user.", 409, request.Path);
      }

      if (tool->IsUpdating())
        Throw("BUSY", "Conflict", "TotalSegmentator is running already.", 409, request.Path);
    });

    auto job = server->CreateJob("Run TotalSegmentator");
    job->SetCancelHandler([]() { CancelTasks("TotalSegmentator"); });

    server->Post([job, path = request.Path]() {
      try
      {
        // Held, as the tool can be deactivated while it runs.
        TotalSegmentatorTool::Pointer tool(GetTotalSegmentator(path));

        TotalSegmentatorStartListener::Start(tool, [job, tool](std::exception_ptr error) {
          if (error != nullptr)
          {
            FailJob(*job, error);
            return;
          }

          if (tool->IsUpdating() || tool->IsRunningOffGUIThread())
          {
            job->Fail("TotalSegmentator is still working.");
            return;
          }

          tool->UpdatePreview();

          WhenToolIdle(tool, [job, tool]() {
            if (!tool->LastRunSucceeded())
            {
              job->Fail(tool->GetLastErrorMessage().empty() ? "TotalSegmentator was cancelled." : tool->GetLastErrorMessage());
              return;
            }

            job->Succeed(DescribeTotalSegmentator(tool, false));
          });
        });
      }
      catch (...)
      {
        FailJob(*job, std::current_exception());
      }
    });

    return RestJob::Respond(request, job);
  };
  routes.push_back(totalSegmentatorRun);
#endif

  return routes;
}
