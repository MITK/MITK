/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationRestRoutes.h"
#include "QmitkSegmentationView.h"

#include <mitkSegmentationPluginConfig.h>

#include <mitkCoreServices.h>
#include <mitkErrorResponse.h>
#include <mitkINodeSelectionService.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkIProgressService.h>
#include <mitkIRestServerService.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkMultiLabelPredicateHelper.h>
#include <mitkRenderingManager.h>
#include <mitkRestGuiThread.h>
#include <mitkRestHelpers.h>
#include <mitkRestJob.h>
#include <mitkRestPoints.h>
#include <mitkRestSegmentation.h>
#include <mitkRestSegmentationJobs.h>
#include <mitkRestTime.h>
#include <mitkSegChangeOperationApplier.h>
#include <mitkSegWithPreviewTool.h>
#include <mitkSegmentationHelper.h>
#include <mitkToolManagerProvider.h>

#if MITK_HAS_PYTHON
#include <QmitkVenvProcess.h>
#include <mitkBaseRenderer.h>
#include <mitkProgressTask.h>
#include <mitkPythonHelper.h>
#include <mitkRestPythonSegmentation.h>
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

#include <QString>

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

namespace
{
  const std::string ViewId = "org.mitk.views.segmentation";

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
      mitk::ThrowRestError("WORKBENCH_NOT_AVAILABLE", "Service Unavailable", "The Workbench has no window.", 503, path);

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

  /** The Segmentation view, once ShowView() has opened it. */
  QmitkSegmentationView* GetView(const std::string& path)
  {
    auto* view = dynamic_cast<QmitkSegmentationView*>(GetPage(path)->FindView(QString::fromStdString(ViewId)).GetPointer());

    if (view == nullptr)
      mitk::ThrowRestError("WORKBENCH_NOT_AVAILABLE", "Service Unavailable", "The Segmentation view is not open.", 503, path);

    return view;
  }

  /** Whether a tool that runs in a Python environment of its own has it; nothing for other tools. */
  std::optional<bool> IsInstalled([[maybe_unused]] const mitk::Tool* tool)
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
    auto* activeTool = toolManager->GetActiveTool();

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
      auto json = mitk::DescribeRestNode(node, server);

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
      { "unconfirmed_result", activeTool != nullptr && activeTool->ConfirmBeforeDeactivation() },
      { "tools", tools },
      { "restrictions", mitk::DescribeRestRestrictions(*mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node(ViewId)) }
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

  mitk::MultiLabelSegmentation* GetSegmentation(mitk::DataNode* node, const std::string& path)
  {
    auto* segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());

    if (segmentation == nullptr)
      mitk::ThrowRestError("NOT_A_SEGMENTATION", "Unprocessable Content", "The node \"" + node->GetName() + "\" is no segmentation.", 422, path);

    return segmentation;
  }

  mitk::SegWithPreviewTool* GetPreviewTool(const std::string& path)
  {
    auto* tool = dynamic_cast<mitk::SegWithPreviewTool*>(GetToolManager()->GetActiveTool());

    if (tool == nullptr)
      mitk::ThrowRestError("NO_PREVIEW_TOOL", "Conflict", "No tool with a result to confirm is active.", 409, path);

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

#if MITK_HAS_PYTHON
  mitk::nnInteractiveTool* GetNNInteractive(const std::string& path)
  {
    auto* tool = dynamic_cast<mitk::nnInteractiveTool*>(GetToolManager()->GetActiveTool());

    if (tool == nullptr)
    {
      mitk::ThrowRestError("TOOL_NOT_ACTIVE", "Conflict",
        "nnInteractive is not active. Activate it with PATCH /api/v1/segmentation/session {\"tool\": \"nnInteractive\"}.", 409, path);
    }

    return tool;
  }

  mitk::VoxTellTool* GetVoxTell(const std::string& path)
  {
    auto* tool = dynamic_cast<mitk::VoxTellTool*>(GetToolManager()->GetActiveTool());

    if (tool == nullptr)
    {
      mitk::ThrowRestError("TOOL_NOT_ACTIVE", "Conflict",
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
      mitkThrow() << displayName << " was canceled in the Workbench.";

    if (result == QmitkVenvProcess::StepResult::Failed)
    {
      MITK_ERROR << "VoxTell: " << displayName << " failed:\n" << output.toStdString();
      mitkThrow() << displayName << " failed. The log tells why.";
    }
  }

  /** \param measure Whether the preview labels have voxels, volume_ml and center, which take a pass over them. */
  nlohmann::json DescribeVoxTell(mitk::VoxTellTool* tool, bool measure = false)
  {
    return mitk::DescribeRestVoxTell(tool, GetToolManager(), IsInstalled(tool).value_or(false), measure);
  }

  bool FailIfDeactivated(mitk::RestJob& job, const mitk::Tool* tool)
  {
    return mitk::FailRestJobIfDeactivated(job, GetToolManager(), tool);
  }

  mitk::TotalSegmentatorTool* GetTotalSegmentator(const std::string& path)
  {
    auto* tool = dynamic_cast<mitk::TotalSegmentatorTool*>(GetToolManager()->GetActiveTool());

    if (tool == nullptr)
    {
      mitk::ThrowRestError("TOOL_NOT_ACTIVE", "Conflict",
        "TotalSegmentator is not active. Activate it with PATCH /api/v1/segmentation/session {\"tool\": \"TotalSegmentator\"}.", 409, path);
    }

    return tool;
  }

  mitk::TotalSegmentatorTool* GetInitializedTotalSegmentator(const std::string& path)
  {
    auto* tool = GetTotalSegmentator(path);

    if (!tool->IsInitialized())
    {
      mitk::ThrowRestError("NOT_INITIALIZED", "Conflict",
        "TotalSegmentator has not started yet. Start it with POST /api/v1/segmentation/totalsegmentator/initialize.", 409, path);
    }

    return tool;
  }

  /**
   * \param withTasks Whether to list the tasks, which only GET does, as there are many.
   * \param measure Whether the preview labels have voxels, volume_ml and center, which take a pass over them.
   */
  nlohmann::json DescribeTotalSegmentator(mitk::TotalSegmentatorTool* tool, bool withTasks, bool measure = false)
  {
    return mitk::DescribeRestTotalSegmentator(tool, GetToolManager(), withTasks, measure);
  }
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
    "null, active_label: {value, name} | null, tool: name | null, unconfirmed_result, tools: [{name, usable, "
    "installed}], restrictions: {label_set_preset, label_suggestions_enforced}}}. The "
    "reference is the image that tools segment; tools write into the active label of the segmentation. "
    "unconfirmed_result is true if the active tool holds a result that is not confirmed and that the view would "
    "ask the user about before it discards it; it is false without an active tool. "
    "time_steps are the time points that each time step of the data covers, [{min_timepoint_ms, "
    "max_timepoint_ms}], the maximum excluded; data of a single time step has null bounds, as it shows at any "
    "time point. The "
    "tools are known once the Segmentation view has been open. installed is there for tools that need an "
    "installation of their own, like nnInteractive, VoxTell and TotalSegmentator; the user installs them "
    "through the tool's panel in the Segmentation view. Tools download the models they need on their own, "
    "which GET /api/v1/workbench/tasks shows. restrictions are what the user's preferences demand of "
    "segmentations that the user makes: label_set_preset, the file name of the preset that the view loads into "
    "a new segmentation, or null; label_suggestions_enforced, whether the view takes only the label names of "
    "GET /api/v1/segmentation/label-suggestions. The routes do not enforce them, so a client that works for the "
    "user follows them.";
  getSession.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() { return DescribeSession(*server, request.Path); }));
  };
  routes.push_back(getSession);

  RestRoute getLabelSuggestions;
  getLabelSuggestions.Method = "GET";
  getLabelSuggestions.Path = "/segmentation/label-suggestions";
  getLabelSuggestions.Summary = "Get the label names that the Segmentation view suggests";
  getLabelSuggestions.Description =
    "Returns {data: {names: [name], enforced}}: the label names of the user's preferences, which can be "
    "hundreds, and whether the view takes only those. Needed only to follow enforced suggestions, see "
    "restrictions of GET /api/v1/segmentation/session.";
  getLabelSuggestions.Handler = [server](const RestRequest&) {
    return RestResponse::FromData(CallInGuiThread(*server, []() { return DescribeRestLabelSuggestions(); }));
  };
  routes.push_back(getLabelSuggestions);

  RestRoute patchSession;
  patchSession.Method = "PATCH";
  patchSession.Path = "/segmentation/session";
  patchSession.Summary = "Choose the image, segmentation, active label and tool of the Segmentation view";
  patchSession.Description =
    "Body, all optional and applied in this order: {reference: uid of an image, segmentation: uid of a "
    "segmentation of that image or null, active_label: value, tool: name or null}. Opens the Segmentation "
    "view if needed and selects there as the user would. A tool needs an image and a segmentation; "
    "changing them ends the active tool, as in the view, and discards results it did not confirm. A node that "
    "the view does not take is answered with 422 NOT_ACCEPTED and changes nothing. While the view cannot use "
    "its tools, for example with the segmentation invisible or the render windows not aligned to its slices, "
    "a tool is answered with 409 TOOLS_NOT_USABLE and the reason. An active_label or tool "
    "that is refused leaves what was applied before it applied. After segmentation: null, "
    "the view selects another segmentation of the image if there is one. Other fields are refused. Returns "
    "{data: session} as GET does.";
  patchSession.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    ThrowIfUnknownRestField(body, { "reference", "segmentation", "active_label", "tool" }, "the session", request.Path);

    if (body.contains("active_label") && (!body["active_label"].is_number_unsigned() ||
        body["active_label"].get<std::uint64_t>() > std::numeric_limits<MultiLabelSegmentation::LabelValueType>::max()))
    {
      throw RestException(ErrorResponse::InvalidRequest("\"active_label\" must be the value of a label.", request.Path));
    }

    if (body.contains("tool") && !body["tool"].is_null() && !body["tool"].is_string())
      throw RestException(ErrorResponse::InvalidRequest("\"tool\" must be the name of a tool or null.", request.Path));

    // Resolved before going to the GUI thread; see CallInGuiThread().
    DataNodeHandle reference;
    DataNodeHandle segmentation;

    if (body.contains("reference"))
      reference = mitk::FindRestNode(*server, body["reference"], request.Path);

    if (body.contains("segmentation") && !body["segmentation"].is_null())
      segmentation = mitk::FindRestNode(*server, body["segmentation"], request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* toolManager = GetToolManager();

      // A node that a selector of the view refuses must not reach it: the
      // selector would drop its selection and select another node on its own,
      // which ends the active tool. Hence the checks with the predicates of
      // the selectors before, and those after only as a safeguard.
      const auto throwReferenceNotAccepted = [&]() {
        mitk::ThrowRestError("NOT_ACCEPTED", "Unprocessable Content",
          "The Segmentation view does not take \"" + reference->GetName() + "\" as image to segment.", 422, request.Path);
      };

      const auto throwSegmentationNotAccepted = [&]() {
        mitk::ThrowRestError("NOT_ACCEPTED", "Unprocessable Content",
          "The Segmentation view does not take \"" + segmentation->GetName() + "\" as segmentation of the image. "
          "A segmentation needs the geometry of the image.", 422, request.Path);
      };

      if (reference != nullptr)
      {
        if (!GetSegmentationReferenceImagePredicate()->CheckNode(reference.get()))
          throwReferenceNotAccepted();

        // The segmentation has to fit the new image, which is checked before the image changes.
        if (segmentation != nullptr && !GetMultiLabelSegmentationPredicate(reference->GetData()->GetGeometry())->CheckNode(segmentation.get()))
          throwSegmentationNotAccepted();

        SelectInView("referenceNode", reference.get());

        if (toolManager->GetReferenceData(0) != reference.get())
          throwReferenceNotAccepted();
      }

      if (segmentation != nullptr)
      {
        const auto* currentReference = toolManager->GetReferenceData(0);
        const auto* geometry = currentReference != nullptr && currentReference->GetData() != nullptr
          ? currentReference->GetData()->GetGeometry()
          : nullptr;

        if (!GetMultiLabelSegmentationPredicate(geometry)->CheckNode(segmentation.get()))
          throwSegmentationNotAccepted();

        SelectInView("workingNode", segmentation.get());

        if (toolManager->GetWorkingData(0) != segmentation.get())
          throwSegmentationNotAccepted();
      }
      else if (body.contains("segmentation"))
      {
        // The view may select another segmentation of the image on its own, which the returned session shows.
        SelectInView("workingNode", nullptr);
      }

      if (body.contains("active_label"))
      {
        auto* working = toolManager->GetWorkingData(0);

        if (working == nullptr)
          mitk::ThrowRestError("NO_SEGMENTATION", "Conflict", "Choose a segmentation first.", 409, request.Path);

        const auto value = body["active_label"].get<MultiLabelSegmentation::LabelValueType>();
        auto* segmentation = GetSegmentation(working, request.Path);

        if (!segmentation->ExistLabel(value))
          mitk::ThrowRestError("LABEL_NOT_FOUND", "Not Found", "The segmentation has no label " + std::to_string(value) + ".", 404, request.Path);

        // Through the tool manager, so that the view selects the label as well.
        toolManager->SetActiveWorkingLabel(value);

        // The view applies the label to the segmentation, but only with a render window editor open.
        const auto isActive = [segmentation, value]() {
          const auto* active = segmentation->GetActiveLabel();
          return active != nullptr && active->GetValue() == value;
        };

        if (!isActive())
          segmentation->SetActiveLabel(value);

        if (!isActive())
        {
          const auto* active = segmentation->GetActiveLabel();

          mitk::ThrowRestError("NOT_ACCEPTED", "Unprocessable Content", "The Segmentation view did not make label " +
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
          const auto name = body["tool"].get<std::string>();
          const Tool* tool = nullptr;

          for (const auto& candidate : toolManager->GetTools())
          {
            if (name == candidate->GetName())
              tool = candidate;
          }

          if (tool == nullptr)
            mitk::ThrowRestError("TOOL_NOT_FOUND", "Not Found", "There is no tool \"" + name + "\". GET /api/v1/segmentation/session lists them.", 404, request.Path);

          // The tool manager would make the tool active without setting it up.
          if (const auto reason = GetView(request.Path)->GetToolsUnusableReason(); !reason.empty())
            mitk::ThrowRestError("TOOLS_NOT_USABLE", "Conflict", "The tool \"" + name + "\" cannot be activated now. " + reason, 409, request.Path);

          if (!toolManager->ActivateTool(toolManager->GetToolID(tool)) || toolManager->GetActiveTool() != tool)
          {
            mitk::ThrowRestError("TOOL_NOT_ACTIVATED", "Conflict",
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
    "Body: {reference: uid of the image, name (optional), static (optional, default false), labels "
    "(optional): [name or {name, color: \"#rrggbb\"}]}. With static true and an image of several time steps, "
    "the segmentation has one time step that holds for all of them, as the view creates a static "
    "segmentation; otherwise it has the time steps of the image. Without labels, the segmentation gets one "
    "label with a suggested name. The label set preset and the enforced label suggestions of the user's "
    "preferences, which GET /api/v1/segmentation/session reports as restrictions, do not bind this request: "
    "it loads no preset and takes any label name. The new "
    "segmentation becomes the one the Segmentation view works on, with its first label active, which ends "
    "the active tool and discards results it did not confirm. An image that the view does not take is "
    "answered with 422 NOT_ACCEPTED. Returns "
    "{data: {uid, name, labels: [label]}} with status 201, labels as GET .../labels gives them without measure.";
  create.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (!body.contains("reference"))
      throw RestException(ErrorResponse::InvalidRequest("\"reference\", the uid of an image, is missing.", request.Path));

    if (body.contains("static") && !body["static"].is_boolean())
      throw RestException(ErrorResponse::InvalidRequest("\"static\" must be true or false.", request.Path));

    const auto reference = mitk::FindRestNode(*server, body["reference"], request.Path);
    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto json = CallInGuiThread(*server, [&]() {
      // Before the node exists, as a request that fails must not leave one behind.
      ShowView(request.Path);

      const auto* image = dynamic_cast<const Image*>(reference->GetData());

      if (image == nullptr || image->GetDimension() < 2)
        mitk::ThrowRestError("NOT_AN_IMAGE", "Unprocessable Content", "\"" + reference->GetName() + "\" is no image of two or more dimensions.", 422, request.Path);

      // See PATCH /segmentation/session: an image that the view refuses must not reach its selector.
      if (!GetSegmentationReferenceImagePredicate()->CheckNode(reference.get()))
      {
        mitk::ThrowRestError("NOT_ACCEPTED", "Unprocessable Content",
          "The Segmentation view does not take \"" + reference->GetName() + "\" as image to segment.", 422, request.Path);
      }

      Image::ConstPointer segmentationTemplate = image;

      if (body.value("static", false) && 1 < image->GetTimeSteps())
        segmentationTemplate = SegmentationHelper::GetStaticSegmentationTemplate(image);

      auto newNode = LabelSetImageHelper::CreateNewSegmentationNode(reference.get(), segmentationTemplate, body.value("name", std::string()), dataStorage);
      auto* segmentation = GetSegmentation(newNode, request.Path);

      std::vector<Label::Pointer> labels;

      for (const auto& entry : body.value("labels", nlohmann::json::array()))
      {
        auto label = LabelSetImageHelper::CreateNewLabel(segmentation);
        const auto name = entry.is_string() ? entry.get<std::string>() : entry.value("name", std::string());

        if (!name.empty())
          label->SetName(name);

        if (entry.is_object() && entry.contains("color"))
          label->SetColor(ParseRestColorOrThrow(entry["color"], request.Path));

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
      dataStorage->Add(newNode, reference.get());

      SelectInView("referenceNode", reference.get());
      SelectInView("workingNode", newNode);

      auto answer = DescribeRestNode(newNode, *server);
      answer["labels"] = DescribeRestLabels(segmentation);
      return answer;
    });

    return RestResponse::FromData(json, 201);
  };
  routes.push_back(create);

  RestRoute labels;
  labels.Method = "GET";
  labels.Path = "/segmentation/segmentations/:uid/labels";
  labels.Summary = "List the labels of a segmentation, optionally with their volumes";
  labels.Description =
    "Returns {data: [{value, name, color, visible, locked, group}]}. Query parameters: measure (optional, true "
    "or false), true adds voxels, volume_ml and center to each label, which costs a pass over the segmentation, "
    "so ask for it only when the numbers are needed; timepoint_ms (optional, with measure=true only), a time "
    "point in milliseconds, or all for each time step of the segmentation, the selected time point without it. "
    "With measure=true, voxels and volume_ml are those at the time point, and center is the center of the voxels "
    "in millimeters, null for an empty label; with all, each label has time_steps: [{min_timepoint_ms, "
    "max_timepoint_ms, voxels, volume_ml, center}] instead. Centers tell which labels or instances of two "
    "segmentations mark the same finding. voxels is null while a tool writes into the segmentation, and when the "
    "segmentation has no data at the time point.";
  labels.Handler = [server](const RestRequest& request) {
    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);
    const auto measure = IsRestMeasureRequested(request);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      // Reads the selected time point without the query parameter.
      const auto selection = measure ? std::optional<RestTimeSelection>(RestTimeSelection::FromRequest(request)) : std::nullopt;
      return DescribeRestLabels(GetSegmentation(node.get(), request.Path), selection);
    }));
  };
  routes.push_back(labels);

  RestRoute addLabel;
  addLabel.Method = "POST";
  addLabel.Path = "/segmentation/segmentations/:uid/labels";
  addLabel.Summary = "Add a label to a segmentation";
  addLabel.Description =
    "Body: {name (optional; a suggested name otherwise), color (optional, \"#rrggbb\"), group (optional, "
    "the active group otherwise)}. The enforced label suggestions of the user's preferences, which GET "
    "/api/v1/segmentation/session reports as restrictions, do not bind this request: it takes any name. "
    "Undoable. Returns {data: label} with status 201. To segment into it, "
    "make it the active label with PATCH /api/v1/segmentation/session.";
  addLabel.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node.get(), request.Path);

      if (body.contains("group") && !body["group"].is_number_unsigned())
        throw RestException(ErrorResponse::InvalidRequest("\"group\" must be the index of a group.", request.Path));

      const MultiLabelSegmentation::GroupIndexType activeGroup = segmentation->GetActiveLayer();
      const auto group = body.value("group", activeGroup);

      if (group >= segmentation->GetNumberOfGroups())
        mitk::ThrowRestError("GROUP_NOT_FOUND", "Not Found", "The segmentation has no group " + std::to_string(group) + ".", 404, request.Path);

      // Value 0 is no label, so this many labels use every value.
      if (segmentation->GetTotalNumberOfLabels() >= Label::MAX_LABEL_VALUE)
        mitk::ThrowRestError("TOO_MANY_LABELS", "Conflict", "The segmentation cannot hold another label.", 409, request.Path);

      auto label = LabelSetImageHelper::CreateNewLabel(segmentation);

      if (body.contains("name"))
        label->SetName(body["name"].get<std::string>());

      if (body.contains("color"))
        label->SetColor(ParseRestColorOrThrow(body["color"], request.Path));

      SegGroupModifyUndoRedoHelper undoRedo(segmentation, { group }, false, 0, false, true, true);
      auto* added = segmentation->AddLabel(label, group, false);

      if (added == nullptr)
        mitk::ThrowRestError("TOO_MANY_LABELS", "Conflict", "The segmentation cannot hold another label.", 409, request.Path);

      undoRedo.RegisterUndoRedoOperationEvent("Add label \"" + added->GetName() + "\"");

      node->Modified();
      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeRestLabel(added, group);
    }), 201);
  };
  routes.push_back(addLabel);

  RestRoute changeLabel;
  changeLabel.Method = "PATCH";
  changeLabel.Path = "/segmentation/segmentations/:uid/labels/:value";
  changeLabel.Summary = "Rename, recolor, show, hide, lock or unlock a label";
  changeLabel.Description =
    "Body, all optional: {name, color: \"#rrggbb\", visible, locked}. Locked labels are kept when tools "
    "write into the segmentation. Renaming and recoloring are undoable. Other fields are refused, as is a "
    "field of the wrong type, and then nothing changes. Returns {data: label}.";
  changeLabel.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    ThrowIfUnknownRestField(body, { "name", "color", "visible", "locked" }, "a label", request.Path);

    if (body.contains("name") && !body["name"].is_string())
      throw RestException(ErrorResponse::InvalidRequest("\"name\" must be a string.", request.Path));

    if (body.contains("visible") && !body["visible"].is_boolean())
      throw RestException(ErrorResponse::InvalidRequest("\"visible\" must be true or false.", request.Path));

    if (body.contains("locked") && !body["locked"].is_boolean())
      throw RestException(ErrorResponse::InvalidRequest("\"locked\" must be true or false.", request.Path));

    const auto newColor = body.contains("color") ? std::optional<Color>(ParseRestColorOrThrow(body["color"], request.Path)) : std::nullopt;
    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node.get(), request.Path);
      auto label = GetRestLabel(segmentation, request);
      const auto group = segmentation->GetGroupIndexOfLabel(label->GetValue());

      if (body.contains("name") || newColor.has_value())
      {
        const auto name = body.contains("name") ? body["name"].get<std::string>() : label->GetName();
        const auto color = newColor.value_or(label->GetColor());

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

      node->Modified();
      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeRestLabel(label, group);
    }));
  };
  routes.push_back(changeLabel);

  RestRoute removeLabel;
  removeLabel.Method = "DELETE";
  removeLabel.Path = "/segmentation/segmentations/:uid/labels/:value";
  removeLabel.Summary = "Remove a label and its contents";
  removeLabel.Description =
    "Undoable. Removing the active label can leave the segmentation without an active label, which GET "
    "/api/v1/segmentation/session tells. Returns {data: [label]}, the labels that remain, as GET .../labels "
    "gives them without measure.";
  removeLabel.Handler = [server](const RestRequest& request) {
    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node.get(), request.Path);
      const auto label = GetRestLabel(segmentation, request);
      const auto name = label->GetName();

      SegGroupModifyUndoRedoHelper undoRedo(segmentation, { segmentation->GetGroupIndexOfLabel(label->GetValue()) }, true);
      segmentation->RemoveLabels({ label->GetValue() });
      undoRedo.RegisterUndoRedoOperationEvent("Remove label \"" + name + "\"");

      node->Modified();
      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeRestLabels(segmentation);
    }));
  };
  routes.push_back(removeLabel);

  RestRoute splitLabel;
  splitLabel.Method = "POST";
  splitLabel.Path = "/segmentation/segmentations/:uid/labels/:value/split";
  splitLabel.Summary = "Split a label into one instance per separate part";
  splitLabel.Description =
    "For a label that stands for several separate findings, like the lung nodules that TotalSegmentator or "
    "VoxTell put into one label. The label keeps its largest part; every further part becomes a new instance, "
    "a label with the same name, color and group but a value of its own. Voxels touching at a face, an edge or "
    "a corner belong to the same part; no voxel is added or removed. Query parameter: timepoint_ms (optional), "
    "the time point to split, the selected time point without it; other time steps stay with the label. "
    "Undoable. Returns {data: [label]}, the label and its new instances, largest first, as GET .../labels gives "
    "them without measure, so without their voxels, volumes and centers, which GET .../labels?measure=true tells; "
    "only the label if it has no separate parts. Status 409 TOO_MANY_LABELS if the segmentation cannot hold the "
    "new instances.";
  splitLabel.Handler = [server](const RestRequest& request) {
    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node.get(), request.Path);
      const auto label = GetRestLabel(segmentation, request);
      const auto selection = RestTimeSelection::FromRequest(request);

      if (selection.All)
        throw RestException(ErrorResponse::InvalidRequest("A label is split at one time point, timepoint_ms cannot be all.", request.Path));

      const auto timeSteps = selection.GetTimeSteps(segmentation);

      if (timeSteps.empty())
      {
        mitk::ThrowRestError("NO_DATA_AT_TIME_POINT", "Unprocessable Content", "The segmentation has no data at " +
          std::to_string(selection.TimePoint) + " ms.", 422, request.Path);
      }

      const auto name = label->GetName();
      SegGroupModifyUndoRedoHelper undoRedo(segmentation, { segmentation->GetGroupIndexOfLabel(label->GetValue()) },
        false, timeSteps.front(), false, false, true);
      MultiLabelSegmentation::LabelValueVectorType values;

      try
      {
        values = segmentation->SplitLabelIntoConnectedComponents(label->GetValue(), timeSteps.front());
      }
      catch (const LabelValuesExhaustedException& e)
      {
        mitk::ThrowRestError("TOO_MANY_LABELS", "Conflict", e.GetDescription(), 409, request.Path);
      }

      if (1 < values.size())
        undoRedo.RegisterUndoRedoOperationEvent("Split label \"" + name + "\" into " + std::to_string(values.size()) + " instances");

      node->Modified();
      RenderingManager::GetInstance()->RequestUpdateAll();

      auto instances = nlohmann::json::array();

      for (const auto value : values)
      {
        if (const auto instance = segmentation->GetLabel(value); instance.IsNotNull())
          instances.push_back(DescribeRestLabel(instance, segmentation->GetGroupIndexOfLabel(value)));
      }

      return instances;
    }));
  };
  routes.push_back(splitLabel);

  RestRoute groups;
  groups.Method = "GET";
  groups.Path = "/segmentation/segmentations/:uid/groups";
  groups.Summary = "List the groups of a segmentation with their labels";
  groups.Description =
    "A group holds labels that do not overlap; labels in different groups may overlap. Returns {data: "
    "[group]}, a group being {index, name, display_name, labels: [{value, name, color, visible, locked, "
    "group}]}; name is empty if the group has none, display_name is what the Segmentation view shows.";
  groups.Handler = [server](const RestRequest& request) {
    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      return DescribeRestGroups(GetSegmentation(node.get(), request.Path));
    }));
  };
  routes.push_back(groups);

  RestRoute group;
  group.Method = "GET";
  group.Path = "/segmentation/segmentations/:uid/groups/:index";
  group.Summary = "Get a group of a segmentation with its labels";
  group.Description = "Returns {data: group} as GET .../groups lists it.";
  group.Handler = [server](const RestRequest& request) {
    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      auto* segmentation = GetSegmentation(node.get(), request.Path);
      return DescribeRestGroup(segmentation, GetRestGroupIndex(segmentation, request));
    }));
  };
  routes.push_back(group);

  RestRoute addGroup;
  addGroup.Method = "POST";
  addGroup.Path = "/segmentation/segmentations/:uid/groups";
  addGroup.Summary = "Add an empty group to a segmentation";
  addGroup.Description =
    "Body, optional: {name}; a name that is no string is refused and nothing is added. Undoable. Returns "
    "{data: group} with status 201. Add labels to it with POST "
    ".../labels and its index as group. Tools that add labels of their own, like TotalSegmentator and "
    "VoxTell, add them to the group of the active label: to fill this group with them, add a label to it and "
    "make that label active first.";
  addGroup.Handler = [server](const RestRequest& request) {
    const auto body = request.Body.empty() ? nlohmann::json::object() : request.GetJsonBody();

    if (body.contains("name") && !body["name"].is_string())
      throw RestException(ErrorResponse::InvalidRequest("\"name\" must be a string.", request.Path));

    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node.get(), request.Path);
      const auto added = segmentation->AddGroup();

      if (body.contains("name"))
        segmentation->SetGroupName(added, body["name"].get<std::string>());

      SegGroupInsertUndoRedoHelper undoRedo(segmentation, { added }, false, true);
      undoRedo.RegisterUndoRedoOperationEvent("Insert new group \"" + LabelSetImageHelper::CreateDisplayGroupName(segmentation, added) + "\"");

      node->Modified();
      return DescribeRestGroup(segmentation, added);
    }), 201);
  };
  routes.push_back(addGroup);

  RestRoute changeGroup;
  changeGroup.Method = "PATCH";
  changeGroup.Path = "/segmentation/segmentations/:uid/groups/:index";
  changeGroup.Summary = "Rename a group of a segmentation";
  changeGroup.Description =
    "Body: {name}; an empty name gives the group its default display name back. Other fields are refused. "
    "Undoable. Returns {data: group}.";
  changeGroup.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    ThrowIfUnknownRestField(body, { "name" }, "a group", request.Path);

    if (!body.contains("name") || !body["name"].is_string())
      throw RestException(ErrorResponse::InvalidRequest("\"name\", the new name of the group, is missing.", request.Path));

    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node.get(), request.Path);
      const auto index = GetRestGroupIndex(segmentation, request);

      SegGroupModifyUndoRedoHelper undoRedo(segmentation, { index }, true, 0, true, true);
      segmentation->SetGroupName(index, body["name"].get<std::string>());
      undoRedo.RegisterUndoRedoOperationEvent("Rename group #" + std::to_string(index));

      node->Modified();
      return DescribeRestGroup(segmentation, index);
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
    const auto node = mitk::FindRestNode(*server, request.PathParams.at("uid"), request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* segmentation = GetSegmentation(node.get(), request.Path);
      const auto index = GetRestGroupIndex(segmentation, request);

      // MultiLabelSegmentation::RemoveGroup() would also remove the last one.
      if (segmentation->GetNumberOfGroups() < 2)
        mitk::ThrowRestError("LAST_GROUP", "Conflict", "A segmentation keeps at least one group, and this is its last.", 409, request.Path);

      const auto name = LabelSetImageHelper::CreateDisplayGroupName(segmentation, index);

      SegGroupRemoveUndoRedoHelper undoRedo(segmentation, { index });
      segmentation->RemoveGroup(index);
      undoRedo.RegisterUndoRedoOperationEvent("Remove group \"" + name + "\"");

      node->Modified();
      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeRestGroups(segmentation);
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
    "Undoable. Answers 409 NOTHING_TO_CONFIRM if the tool has no result, as after a confirm. "
    "Returns {data: {segmentation: {uid, name}, labels: [label]}} as GET .../labels gives them without measure; "
    "check with GET .../labels?measure=true that the label is filled. If the tool has to compute the result first, for all_time_steps "
    "with a tool that previews only the time step on display, or for an active label that changed since the "
    "result was computed, which takes as long as that many runs, the confirm runs as a job: query parameter "
    "wait (seconds) waits for it, and the job's result is the answer. If that computation fails, is canceled in "
    "the Workbench, or the tool is deactivated meanwhile, nothing is confirmed: the confirm answers 500 "
    "CONFIRM_FAILED, or its job fails, with the reason. A result whose labels the segmentation cannot hold "
    "next to its own (at most 65535) answers 409 TOO_MANY_LABELS, with nothing confirmed.";
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
      ThrowIfRestToolBusy(tool, request.Path);

      // An empty preview would erase the label it is transferred to, for example on a repeated confirm.
      if (!tool->HasResultToConfirm())
      {
        mitk::ThrowRestError("NOTHING_TO_CONFIRM", "Conflict",
          std::string(tool->GetName()) + " has no result to confirm. A confirmed result is not confirmed again.", 409, request.Path);
      }

      // The options are those of this confirm only. Left in the tool, they
      // would be what its panel shows as the choice of the user.
      const auto restoreOptions = [tool, mergeStyle = tool->GetMergeStyle(), overwriteStyle = tool->GetOverwriteStyle(),
                                   createAllTimeSteps = tool->GetCreateAllTimeSteps()]() {
        tool->SetMergeStyle(mergeStyle);
        tool->SetOverwriteStyle(overwriteStyle);
        tool->SetCreateAllTimeSteps(createAllTimeSteps);
      };

      tool->SetMergeStyle(body.value("merge", false) ? MultiLabelSegmentation::MergeStyle::Merge : MultiLabelSegmentation::MergeStyle::Replace);
      tool->SetOverwriteStyle(body.value("ignore_locks", false) ? MultiLabelSegmentation::OverwriteStyle::IgnoreLocks : MultiLabelSegmentation::OverwriteStyle::RegardLocks);
      tool->SetCreateAllTimeSteps(allTimeSteps);

      // Held, as the tool may deactivate itself after confirming.
      const DataNode::Pointer target = tool->GetTargetSegmentationNode();
      const auto describe = [server, target, path = request.Path]() {
        RenderingManager::GetInstance()->RequestUpdateAll();

        return nlohmann::json{
          { "segmentation", DescribeRestNode(target, *server) },
          { "labels", target != nullptr ? DescribeRestLabels(GetSegmentation(target, path)) : nlohmann::json::array() }
        };
      };

      try
      {
        tool->ConfirmSegmentation();
      }
      catch (const LabelValuesExhaustedException& e)
      {
        // Thrown before anything is transferred.
        restoreOptions();
        mitk::ThrowRestError("TOO_MANY_LABELS", "Conflict",
          std::string("The segmentation cannot hold the labels of the result: ") + e.GetDescription(), 409, request.Path);
      }
      catch (...)
      {
        restoreOptions();
        throw;
      }

      if (!tool->IsUpdating())
      {
        restoreOptions();

        // An update before the transfer that failed reports it to the user only.
        if (const auto failure = GetRestConfirmationFailure(*tool); !failure.empty())
          mitk::ThrowRestError("CONFIRM_FAILED", "Internal Server Error", failure, 500, request.Path);

        return Outcome{ describe(), nullptr };
      }

      // The transfer follows the update.
      auto job = server->CreateJob("Confirm the result");
      job->SetCancelHandler([name = std::string(tool->GetName())]() { CancelTasks(name); });

      WhenRestToolIdle(tool, [job, tool, describe, restoreOptions]() {
        restoreOptions();

        if (const auto failure = GetRestConfirmationFailure(*tool); !failure.empty())
        {
          job->Fail(failure);
          return;
        }

        try
        {
          job->Succeed(describe());
        }
        catch (...)
        {
          FailRestJob(*job, std::current_exception());
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
  RestRoute nnInteractiveState;
  nnInteractiveState.Method = "GET";
  nnInteractiveState.Path = "/segmentation/nninteractive";
  nnInteractiveState.Summary = "Get the state of nnInteractive, the interactive segmentation tool";
  nnInteractiveState.Description =
    "Needs nnInteractive as active tool. Returns {data: {session_running, session_starting, remote, "
    "prompt_types, can_undo, auto_zoom, auto_refine, preview_voxels}}; session_starting is true while a start "
    "is under way, whoever began it; preview_voxels is the size of the unconfirmed result.";
  nnInteractiveState.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() { return DescribeRestNNInteractive(GetNNInteractive(request.Path)); }));
  };
  routes.push_back(nnInteractiveState);

  RestRoute nnInteractiveOptions;
  nnInteractiveOptions.Method = "PATCH";
  nnInteractiveOptions.Path = "/segmentation/nninteractive";
  nnInteractiveOptions.Summary = "Change the options of nnInteractive";
  nnInteractiveOptions.Description =
    "As the checkboxes of its panel. Body, all optional: {auto_zoom: true|false, auto_refine: true|false}. "
    "auto_zoom widens the region the model looks at when a result reaches its edge, for large structures, "
    "at the cost of speed; it needs a GPU. auto_refine runs the model on the content of a label when it "
    "becomes the target of the session, instead of taking the content as it is until the next prompt. "
    "Other fields are refused. While a prompt computes or a session starts, the options cannot change and "
    "the request gets 409 BUSY. Returns {data: state} as GET does.";
  nnInteractiveOptions.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    ThrowIfUnknownRestField(body, { "auto_zoom", "auto_refine" }, "the options of nnInteractive", request.Path);

    for (const auto* key : { "auto_zoom", "auto_refine" })
    {
      if (body.contains(key) && !body[key].is_boolean())
        throw RestException(ErrorResponse::InvalidRequest(std::string("\"") + key + "\" must be true or false.", request.Path));
    }

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetNNInteractive(request.Path);
      ThrowIfRestToolBusy(tool, request.Path);

      if (body.contains("auto_zoom"))
      {
        const auto autoZoom = body["auto_zoom"].get<bool>();

        if (autoZoom && tool->GetBackend() == nnInteractive::Backend::CPU)
          mitk::ThrowRestError("NOT_AVAILABLE", "Conflict", "Auto-zoom is not available with the CPU backend.", 409, request.Path);

        tool->SetAutoZoom(autoZoom);
      }

      if (body.contains("auto_refine"))
        tool->SetAutoRefine(body["auto_refine"].get<bool>());

      return DescribeRestNNInteractive(tool);
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
    "it. The job's result is the state as GET /api/v1/segmentation/nninteractive gives it. A start that the "
    "user began in the panel is joined: the job ends with it, and fails if it did not succeed. The job ends "
    "once the session has taken up the content of the active label, if it has any, so the state it gives "
    "includes that. The session works "
    "on the image and time step chosen now; changing them ends it. A session that runs already is kept, with "
    "its prompts and result: the job succeeds at once, or, while the session takes up the active label or a prompt "
    "computes, once that has ended; it fails if the session is ended meanwhile. To start anew, DELETE the session "
    "first.";
  startSession.Handler = [server, start](const RestRequest& request) {
    // A running session is kept; ready tells whether its state is final already.
    const auto kept = CallInGuiThread(*server, [&]() -> std::optional<std::pair<nlohmann::json, bool>> {
      ShowView(request.Path);
      auto* tool = GetNNInteractive(request.Path);

      if (!IsInstalled(tool).value_or(false))
      {
        mitk::ThrowRestError("TOOL_NOT_INSTALLED", "Conflict",
          "nnInteractive is not installed. Ask the user to install it with the Initialize button of the nnInteractive panel.", 409, request.Path);
      }

      // A new start would end the session and discard its prompts and result.
      if (!tool->IsSessionRunning())
        return std::nullopt;

      // Not while the update that seeds it runs, nor while a DELETE waits for a
      // prompt to stop before it ends the session.
      const bool ready = !tool->IsUpdating() && !tool->IsRunningOffGUIThread() && !tool->IsSessionEnding();
      return std::make_pair(DescribeRestNNInteractive(tool), ready);
    });

    // A job in any case, so that clients read one answer.
    if (kept.has_value())
    {
      auto keptJob = server->CreateJob("Start an nnInteractive session");

      if (kept->second)
      {
        keptJob->Succeed(kept->first);
      }
      else
      {
        server->Post([keptJob, path = request.Path]() {
          try
          {
            SucceedRestJobWhenSessionSeeded(keptJob, nnInteractiveTool::Pointer(GetNNInteractive(path)));
          }
          catch (...)
          {
            FailRestJob(*keptJob, std::current_exception());
          }
        });
      }

      return RestJob::Respond(request, keptJob);
    }

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
      // Canceled before this ran: the cancel found no task to stop.
      if (job->HasEnded())
        return;

      try
      {
        // Held, as the tool can be deactivated while the start runs.
        nnInteractiveTool::Pointer tool(GetNNInteractive(path));

        if (tool->IsSessionStarting())
        {
          WhenRestStartEnded(job, [tool]() { return tool->IsSessionStarting(); }, [job, tool]() {
            // The panel does not tell a cancel from a failure.
            if (tool->IsSessionRunning())
              SucceedRestJobWhenSessionSeeded(job, tool);
            else
              job->Fail("The start in the panel was canceled in the Workbench or failed; ask the user how to go on.");
          });

          return;
        }

        // Started meanwhile, in the panel or by another request.
        if (tool->IsSessionRunning())
        {
          SucceedRestJobWhenSessionSeeded(job, tool);
          return;
        }

        auto task = std::make_shared<ProgressTask>("Initializing nnInteractive", ProgressTask::Indeterminate, true);

        {
          std::lock_guard<std::mutex> lock(start->Mutex);
          start->TaskId = task->GetId();
        }

        tool->StartSession(task, [job, task, tool](std::exception_ptr error) {
          if (error != nullptr)
          {
            FailRestJob(*job, error);
            return;
          }

          task->Finish();
          SucceedRestJobWhenSessionSeeded(job, tool);
        });
      }
      catch (...)
      {
        FailRestJob(*job, std::current_exception());
      }
    });

    return RestJob::Respond(request, job);
  };
  routes.push_back(startSession);

  RestRoute endSession;
  endSession.Method = "DELETE";
  endSession.Path = "/segmentation/nninteractive/session";
  endSession.Summary = "End the nnInteractive session, discarding unconfirmed results";
  endSession.Description =
    "As unchecking its Initialize button. While the session starts or a prompt computes, this only asks them to "
    "stop, and the session ends once they have, so the state returned can still say session_running: true. "
    "Returns {data: state} as GET /api/v1/segmentation/nninteractive does.";
  endSession.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetNNInteractive(request.Path);
      tool->AbortSession();
      return DescribeRestNNInteractive(tool);
    }));
  };
  routes.push_back(endSession);

  RestRoute addPrompt;
  addPrompt.Method = "POST";
  addPrompt.Path = "/segmentation/nninteractive/prompts";
  addPrompt.Summary = "Prompt nnInteractive with a point, box, scribble or lasso";
  addPrompt.Description =
    "Needs a running session (409 NOT_INITIALIZED without one). Body: {type: point|box|scribble|lasso, positive: true (the structure, "
    "default) or false (what does not belong to it), and the points: either pixels: [[x, y], ...] of a "
    "screenshot of a 2D window with window, image_size and view_state, as GET /api/v1/rendering/windows/{id}/screenshot "
    "describes it (409 VIEW_CHANGED if the view has changed since: take a new screenshot), or points: [[x, y, z], ...] in millimeters, all in voxels of one slice of the image, where they are drawn}. A point "
    "needs one point; a box two opposite corners; a scribble a line of points, painted with the brush of "
    "the panel, three pixels wide; a lasso a closed contour of at least three points, whose inside is the "
    "prompt. A scribble or lasso takes at most 100 points; a few points along the structure are enough. "
    "A prompt that is refused answers 422 PROMPT_FAILED and starts no job. The prompt is drawn as if the user had drawn it with the mouse in that window, and the 2D "
    "windows show it. Prompts add up and refine the unconfirmed result, which POST "
    "/api/v1/segmentation/confirm writes into the active label. The model computes the prompt as a job: "
    "query parameter wait (seconds) waits for it, and the job's result is the state as GET "
    "/api/v1/segmentation/nninteractive gives it. One prompt at a time: a prompt while one computes "
    "gets 409. Canceling the job of a prompt ends the session and discards its prompts and the unconfirmed "
    "result.";
  addPrompt.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto type = ParseRestInteractionType(body, request.Path);
    const auto promptType = body.value("positive", true) ? nnInteractive::PromptType::Positive : nnInteractive::PromptType::Negative;

    const auto job = CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      nnInteractiveTool::Pointer tool(GetNNInteractive(request.Path));

      if (!tool->IsSessionRunning())
      {
        mitk::ThrowRestError("NOT_INITIALIZED", "Conflict",
          "nnInteractive has no session. Start one with POST /api/v1/segmentation/nninteractive/session.", 409, request.Path);
      }

      ThrowIfRestToolBusy(tool, request.Path);

      // For a moment after its update, the last prompt has not reported to its job yet.
      if (tool->IsPromptPending())
        mitk::ThrowRestError("BUSY", "Conflict", "nnInteractive is still computing a prompt. Wait for its job to end.", 409, request.Path);

      const auto points = GetRestWorldPoints(body, request.Path);

      // The prompt is drawn in the plane of the window that the pixels were
      // drawn in, as with the mouse. Points in millimeters come first, as in
      // GetRestWorldPoints(), which checks the window only for pixels.
      const PlaneGeometry* plane = !body.contains("points") && body.contains("pixels")
        ? GetRestRenderer(body["window"].get<std::string>(), request.Path)->GetCurrentWorldPlaneGeometry()
        : nullptr;

      // The job exists once the tool has taken the prompt, which can have failed by then.
      const auto outcome = std::make_shared<RestPromptOutcome>();

      try
      {
        // The tool keeps the callback, so a reference to the tool in it would
        // keep the tool alive.
        tool->AddPrompt(type, points, promptType, plane, [outcome, tool = tool.GetPointer()](std::exception_ptr error) {
          outcome->Ended = true;
          outcome->Error = error;

          if (error == nullptr)
            outcome->Result = DescribeRestNNInteractive(tool);

          outcome->ReportToJob();
        });
      }
      catch (const Exception& e)
      {
        mitk::ThrowRestError("PROMPT_FAILED", "Unprocessable Content", e.GetDescription(), 422, request.Path);
      }

      outcome->Job = server->CreateJob("Prompt nnInteractive");
      outcome->Job->SetCancelHandler([]() { CancelTasks("nnInteractive"); });
      outcome->Job->SetCancelAlsoDiscards("the nnInteractive session with all its prompts and the unconfirmed result");
      outcome->ReportToJob();

      return outcome->Job;
    });

    return RestJob::Respond(request, job);
  };
  routes.push_back(addPrompt);

  RestRoute undoPrompt;
  undoPrompt.Method = "POST";
  undoPrompt.Path = "/segmentation/nninteractive/undo";
  undoPrompt.Summary = "Undo the last nnInteractive prompt";
  undoPrompt.Description =
    "nnInteractive undoes one prompt only: after an undo, the next prompt is undoable again. Returns {data: "
    "state}, or 409 if there is nothing to undo, if the session cannot undo, or while a prompt computes.";
  undoPrompt.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetNNInteractive(request.Path);

      // The tool ignores an undo while it works, which must not read as done.
      ThrowIfRestToolBusy(tool, request.Path);

      if (tool->IsSessionRunning() && !tool->SupportsUndo())
        mitk::ThrowRestError("NOT_AVAILABLE", "Conflict", "The nnInteractive session does not support undo.", 409, request.Path);

      if (!tool->CanUndo())
        mitk::ThrowRestError("NOTHING_TO_UNDO", "Conflict", "There is no prompt to undo.", 409, request.Path);

      tool->UndoLastInteraction();
      return DescribeRestNNInteractive(tool);
    }));
  };
  routes.push_back(undoPrompt);

  RestRoute reset;
  reset.Method = "POST";
  reset.Path = "/segmentation/nninteractive/reset";
  reset.Summary = "Discard all nnInteractive prompts and the unconfirmed result";
  reset.Description = "The session keeps running. Returns {data: state}, or 409 while a prompt computes.";
  reset.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetNNInteractive(request.Path);

      // The tool ignores a reset while it works, which must not read as done.
      ThrowIfRestToolBusy(tool, request.Path);

      tool->ResetInteractions();
      RenderingManager::GetInstance()->RequestUpdateAll();
      return DescribeRestNNInteractive(tool);
    }));
  };
  routes.push_back(reset);

  RestRoute voxTell;
  voxTell.Method = "GET";
  voxTell.Path = "/segmentation/voxtell";
  voxTell.Summary = "Get the state of VoxTell, which segments structures named in text";
  voxTell.Description =
    "Needs VoxTell as active tool. Query parameter: measure (optional, true or false), true adds voxels, "
    "volume_ml and center to the preview labels, which costs a pass over the preview. Returns {data: {installed, "
    "model_loaded, model_loading, prompts, prompts_without_result, error, preview_labels: [label]}}, "
    "model_loading being true while the model is being prepared, whoever began it, and preview_labels being the "
    "unconfirmed result, one label per prompt, as GET .../labels describes labels but without value: group is the "
    "group that the confirm adds them to, that of the active label unless create_groups put a label into another "
    "one, which the confirm creates if the segmentation does not have it, and they get their values with the "
    "confirm. voxels, volume_ml and center are those of GET .../labels at the time point of the run, which the "
    "confirm transfers, also after another time point was selected.";
  voxTell.Handler = [server](const RestRequest& request) {
    const auto measure = IsRestMeasureRequested(request);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() { return DescribeVoxTell(GetVoxTell(request.Path), measure); }));
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
        mitk::ThrowRestError("MODEL_NOT_LOADED", "Conflict",
          "VoxTell has no model loaded. Load it with POST /api/v1/segmentation/voxtell/model.", 409, request.Path);
      }

      // Asks the worker, whose calls must not overlap with those of a run.
      ThrowIfRestToolBusy(tool, request.Path);

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
    "loads it. Runs as a job: query parameter wait (seconds) waits for it. If the model is loaded already, the "
    "job has succeeded when the answer comes. A start that the user began in the panel is joined: the job ends "
    "with it, and fails if it did not succeed. The job's result is the state as GET /api/v1/segmentation/voxtell "
    "gives it without measure.";
  voxTellModel.Handler = [server](const RestRequest& request) {
    const auto loaded = CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetVoxTell(request.Path);

      if (!IsInstalled(tool).value_or(false))
      {
        mitk::ThrowRestError("TOOL_NOT_INSTALLED", "Conflict",
          "VoxTell is not installed. Ask the user to install it with the Initialize button of the VoxTell panel.", 409, request.Path);
      }

      return tool->IsModelLoaded() ? std::optional<nlohmann::json>(DescribeVoxTell(tool)) : std::nullopt;
    });

    auto job = server->CreateJob("Load the VoxTell model");

    if (loaded.has_value())
    {
      job->Succeed(*loaded);
      return RestJob::Respond(request, job);
    }

    job->SetCancelHandler([]() { CancelTasks("VoxTell"); });

    server->Post([job, path = request.Path]() {
      // Canceled before this ran: the cancel found no task to stop.
      if (job->HasEnded())
        return;

      try
      {
        // Held, as the tool can be deactivated while the worker loads.
        VoxTellTool::Pointer tool = GetVoxTell(path);

        if (tool->IsInitializing())
        {
          WhenRestStartEnded(job, [tool]() { return tool->IsInitializing(); }, [job, tool]() {
            // The panel does not tell a cancel from a failure.
            if (tool->IsModelLoaded())
              job->Succeed(DescribeVoxTell(tool));
            else
              job->Fail("The start in the panel was canceled in the Workbench or failed; ask the user how to go on.");
          });

          return;
        }

        tool->Initialize(
          [job](const std::vector<VoxTell::RepoFiles>& files) {
            // A cancel between two steps finds no task to stop.
            if (job->HasEnded())
              mitkThrow() << "The job has ended.";

            DownloadForVoxTell("Downloading VoxTell model", files);

            if (job->HasEnded())
              mitkThrow() << "The job has ended.";
          },
          [job, tool](std::exception_ptr error, bool) {
            if (job->HasEnded() || FailIfDeactivated(*job, tool))
              return;

            if (error != nullptr)
              FailRestJob(*job, error);
            else
              job->Succeed(DescribeVoxTell(tool));
          });
      }
      catch (...)
      {
        FailRestJob(*job, std::current_exception());
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
    "own)}. With create_groups false, all results go into the group of the active label: a later prompt takes "
    "the voxels of an earlier one where they overlap, and the confirm overwrites the labels of that group that "
    "are not locked where the results cover them. White space around and within a prompt is simplified, and a "
    "prompt given twice counts once, whatever its case. "
    "The VoxTell panel shows the prompts. Prompts VoxTell does not know need its text model, which is "
    "downloaded first if it is missing (about 8 GB) and loaded, which takes a while. Runs as a job: query "
    "parameter wait (seconds) waits for it. The job's result is the state as GET "
    "/api/v1/segmentation/voxtell gives it without measure; POST /api/v1/segmentation/confirm then adds one label per "
    "prompt to the segmentation.";
  voxTellPrompts.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto list = body.value("prompts", nlohmann::json::array());

    if (!list.is_array() || std::any_of(list.begin(), list.end(), [](const auto& item) { return !item.is_string(); }))
      throw RestException(ErrorResponse::InvalidRequest("\"prompts\" must be a list of strings.", request.Path));

    // As the panel reads its lines: a known prompt with white space around it
    // would count as unknown and need the text model, and a prompt given
    // twice would make two labels.
    std::vector<std::string> prompts;
    std::set<QString> seen;

    for (const auto& item : list)
    {
      const auto prompt = QString::fromStdString(item.get<std::string>()).simplified();

      if (!prompt.isEmpty() && seen.insert(prompt.toLower()).second)
        prompts.push_back(prompt.toStdString());
    }

    if (prompts.empty())
      throw RestException(ErrorResponse::InvalidRequest("\"prompts\" must list at least one structure.", request.Path));

    const bool createGroups = body.value("create_groups", false);

    CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetVoxTell(request.Path);

      if (!tool->IsModelLoaded())
      {
        mitk::ThrowRestError("MODEL_NOT_LOADED", "Conflict",
          "VoxTell has no model loaded. Load it with POST /api/v1/segmentation/voxtell/model.", 409, request.Path);
      }

      ThrowIfRestToolBusy(tool, request.Path);
    });

    auto job = server->CreateJob("Segment with VoxTell");
    job->SetCancelHandler([]() { CancelTasks("VoxTell"); });

    server->Post([job, prompts, createGroups, path = request.Path]() {
      // Canceled before this ran: the cancel found no task to stop.
      if (job->HasEnded())
        return;

      try
      {
        // Held, as the tool can be deactivated while the model computes.
        VoxTellTool::Pointer tool(GetVoxTell(path));

        // The tool may have begun to work since the request was checked, and
        // what follows asks its worker, whose calls must not overlap.
        ThrowIfRestToolBusy(tool, path);

        const auto unknownPrompts = tool->GetPromptsWithoutPrecomputedEmbedding(prompts);

        // The run, once the prompts are known. The update ends after this
        // has returned.
        const auto run = [job, tool, prompts, createGroups]() {
          // A cancel between two steps finds no task to stop.
          if (job->HasEnded())
            return;

          if (tool->IsUpdating() || tool->IsRunningOffGUIThread())
          {
            job->Fail("VoxTell is still working.");
            return;
          }

          // Deactivated meanwhile, the tool has no segmentation to work on.
          if (GetToolManager()->GetActiveTool() != tool.GetPointer())
          {
            job->Fail("VoxTell is no longer the active tool.");
            return;
          }

          // Stays in the tool with the prompts, as a confirm that recomputes
          // the result places it again. The panel pushes the user's choice
          // for each run of its own.
          tool->SetPrompts(prompts);
          tool->SetCreateGroupsAsNeeded(createGroups);
          tool->UpdatePreview();

          WhenRestToolIdle(tool, [job, tool]() {
            if (FailIfDeactivated(*job, tool))
              return;

            if (!tool->GetLastErrorMessage().empty())
            {
              job->Fail(tool->GetLastErrorMessage());
              return;
            }

            if (tool->WasLastUpdateCancelled())
            {
              job->Fail("VoxTell was canceled in the Workbench.");
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

        // The download runs the event loop, during which the job can be
        // canceled and the tool deactivated.
        if (job->HasEnded() || FailIfDeactivated(*job, tool))
          return;

        auto task = std::make_shared<ProgressTask>("Processing prompts with the VoxTell text model", ProgressTask::Indeterminate, true);

        tool->EmbedPrompts(unknownPrompts, task, [job, run](std::exception_ptr error) {
          if (error != nullptr)
          {
            FailRestJob(*job, error);
            return;
          }

          run();
        });
      }
      catch (...)
      {
        FailRestJob(*job, std::current_exception());
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
    "Needs TotalSegmentator as active tool. Query parameter: measure (optional, true or false), true adds voxels, "
    "volume_ml and center to the preview labels, which costs a pass over the preview. Returns {data: {installed, "
    "initialized, has_license, device, task, speed, "
    "roi_subset: [class], running, error, preview_labels: [label], tasks: [{name, modality: CT | MR, licensed, "
    "available, speeds: [{name: standard | fast | fastest, resolution_mm}], roi_subset, class_count}]}}. task, speed "
    "and roi_subset are what the next run segments; an empty roi_subset means all classes of the task. available is "
    "false for licensed tasks without a license. The tasks are there once TotalSegmentator has started (initialized). "
    "preview_labels are the unconfirmed result of the last run, as GET .../labels describes labels but without "
    "value: group is the group of the active label, where the confirm adds them, and they get their values with the "
    "confirm; voxels, volume_ml and center are those of GET .../labels at the time point of the run, which the "
    "confirm transfers, also after another time point was selected. Device and "
    "license are settings of the user.";
  totalSegmentator.Handler = [server](const RestRequest& request) {
    const auto measure = IsRestMeasureRequested(request);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() { return DescribeTotalSegmentator(GetTotalSegmentator(request.Path), true, measure); }));
  };
  routes.push_back(totalSegmentator);

  RestRoute totalSegmentatorStart;
  totalSegmentatorStart.Method = "POST";
  totalSegmentatorStart.Path = "/segmentation/totalsegmentator/initialize";
  totalSegmentatorStart.Summary = "Start TotalSegmentator, which learns its tasks";
  totalSegmentatorStart.Description =
    "Needs TotalSegmentator as active tool, installed by the user through its panel. Its panel starts it as well when "
    "it opens. Runs as a job: query parameter wait (seconds) waits for it. If it has started already, the job has "
    "succeeded when the answer comes. The job's result is the state as GET /api/v1/segmentation/totalsegmentator "
    "gives it without measure and without tasks.";
  totalSegmentatorStart.Handler = [server](const RestRequest& request) {
    const auto initialized = CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetTotalSegmentator(request.Path);

      if (!TotalSegmentatorTool::IsInstalled())
      {
        mitk::ThrowRestError("TOOL_NOT_INSTALLED", "Conflict",
          "TotalSegmentator is not installed. Ask the user to install it with the button of the TotalSegmentator panel.", 409, request.Path);
      }

      return tool->IsInitialized() ? std::optional<nlohmann::json>(DescribeTotalSegmentator(tool, false)) : std::nullopt;
    });

    auto job = server->CreateJob("Start TotalSegmentator");

    if (initialized.has_value())
    {
      job->Succeed(*initialized);
      return RestJob::Respond(request, job);
    }

    job->SetCancelHandler([]() { CancelTasks("TotalSegmentator"); });

    server->Post([job, path = request.Path]() {
      // Canceled before this ran: the cancel found no task to stop.
      if (job->HasEnded())
        return;

      try
      {
        // Held, as the tool can be deactivated while it starts.
        TotalSegmentatorTool::Pointer tool(GetTotalSegmentator(path));

        StartRestTotalSegmentator(tool, [job, tool](std::exception_ptr error) {
          // A deactivation aborts the start, whose error then reads as a cancel.
          if (FailIfDeactivated(*job, tool))
            return;

          if (error != nullptr)
          {
            FailRestJob(*job, error);
            return;
          }

          job->Succeed(DescribeTotalSegmentator(tool, false));
        });
      }
      catch (...)
      {
        FailRestJob(*job, std::current_exception());
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
        mitk::ThrowRestError("TOTALSEGMENTATOR_TASK_NOT_FOUND", "Not Found", "TotalSegmentator has no task \"" + name + "\".", 404, request.Path);

      return DescribeRestTotalSegmentatorTask(*task, true);
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
    "models (resolution_mm of the task's speeds), quicker; speed is standard unless given with a task. A request "
    "with task or speed selects all classes of the task again, unless roi_subset, the exact class names of GET "
    ".../tasks/{name}, chooses some: "
    "only tasks with roi_subset true take one, and an empty list chooses all. Other fields are refused. Returns "
    "{data: state} as GET does, without tasks.";
  totalSegmentatorOptions.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    ThrowIfUnknownRestField(body, { "task", "speed", "roi_subset" }, "the options of TotalSegmentator", request.Path);

    if (body.contains("task") && !body["task"].is_string())
      throw RestException(ErrorResponse::InvalidRequest("\"task\" must be the name of a task.", request.Path));

    if (body.contains("roi_subset") && (!body["roi_subset"].is_array() || std::any_of(body["roi_subset"].begin(), body["roi_subset"].end(), [](const auto& item) { return !item.is_string(); })))
      throw RestException(ErrorResponse::InvalidRequest("\"roi_subset\" must be a list of class names.", request.Path));

    const auto speed = body.contains("speed")
      ? std::optional<TotalSegmentatorTool::Speed>(ParseRestSpeed(body["speed"], request.Path))
      : std::nullopt;

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetInitializedTotalSegmentator(request.Path);

      if (tool->IsUpdating())
        mitk::ThrowRestError("BUSY", "Conflict", "TotalSegmentator is running. Wait until the run has ended.", 409, request.Path);

      // A request that is refused changes nothing, also if only its subset is.
      const auto previousTask = tool->GetTask();
      const auto previousSpeed = tool->GetSpeed();
      const auto previousSubset = tool->GetRoiSubset();
      bool taskSet = false;

      try
      {
        if (body.contains("task") || speed.has_value())
        {
          const auto task = body.contains("task") ? body["task"].get<std::string>() : tool->GetTask();
          tool->SetTask(task, speed.value_or(body.contains("task") ? TotalSegmentatorTool::Speed::Standard : tool->GetSpeed()));
          taskSet = true;
        }

        if (body.contains("roi_subset"))
          tool->SetRoiSubset(body["roi_subset"].get<std::vector<std::string>>());
      }
      catch (const Exception& e)
      {
        if (taskSet)
        {
          tool->SetTask(previousTask, previousSpeed);
          tool->SetRoiSubset(previousSubset);
        }

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
    "The job's result is the state as GET /api/v1/segmentation/totalsegmentator gives it without measure and without tasks; POST "
    "/api/v1/segmentation/confirm then adds the labels of preview_labels to the segmentation. preview_labels has a "
    "label for each class found in the image, and is empty if none was.";
  totalSegmentatorRun.Handler = [server](const RestRequest& request) {
    CallInGuiThread(*server, [&]() {
      ShowView(request.Path);
      auto* tool = GetTotalSegmentator(request.Path);

      if (!TotalSegmentatorTool::IsInstalled())
      {
        mitk::ThrowRestError("TOOL_NOT_INSTALLED", "Conflict",
          "TotalSegmentator is not installed. Ask the user to install it with the button of the TotalSegmentator panel.", 409, request.Path);
      }

      if (const auto* task = tool->FindTask(tool->GetTask()); task != nullptr && task->Licensed && !TotalSegmentatorTool::HasLicense())
      {
        mitk::ThrowRestError("LICENSE_REQUIRED", "Conflict",
          "The task \"" + tool->GetTask() + "\" requires a TotalSegmentator license, which the user sets in its settings. "
          "Choose a task that is available, or ask the user.", 409, request.Path);
      }

      if (tool->IsUpdating())
        mitk::ThrowRestError("BUSY", "Conflict", "TotalSegmentator is running already.", 409, request.Path);
    });

    auto job = server->CreateJob("Run TotalSegmentator");
    job->SetCancelHandler([]() { CancelTasks("TotalSegmentator"); });

    server->Post([job, path = request.Path]() {
      // Canceled before this ran: the cancel found no task to stop.
      if (job->HasEnded())
        return;

      try
      {
        // Held, as the tool can be deactivated while it runs.
        TotalSegmentatorTool::Pointer tool(GetTotalSegmentator(path));

        StartRestTotalSegmentator(tool, [job, tool](std::exception_ptr error) {
          // Deactivated while it started, the tool has no segmentation to work
          // on. The deactivation also aborts the start, whose error then reads
          // as a cancel.
          if (FailIfDeactivated(*job, tool))
            return;

          if (error != nullptr)
          {
            FailRestJob(*job, error);
            return;
          }

          // Canceled while TotalSegmentator started: the run is a step of its own.
          if (job->HasEnded())
            return;

          if (tool->IsUpdating() || tool->IsRunningOffGUIThread())
          {
            job->Fail("TotalSegmentator is still working.");
            return;
          }

          tool->UpdatePreview();

          WhenRestToolIdle(tool, [job, tool]() {
            if (FailIfDeactivated(*job, tool))
              return;

            if (!tool->LastRunSucceeded())
            {
              job->Fail(tool->GetLastErrorMessage().empty() ? "TotalSegmentator was canceled in the Workbench." : tool->GetLastErrorMessage());
              return;
            }

            job->Succeed(DescribeTotalSegmentator(tool, false));
          });
        });
      }
      catch (...)
      {
        FailRestJob(*job, std::current_exception());
      }
    });

    return RestJob::Respond(request, job);
  };
  routes.push_back(totalSegmentatorRun);
#endif

  return routes;
}
