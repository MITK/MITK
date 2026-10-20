/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRestApiWorkbenchRoutes.h"

#include <mitkCoreServices.h>
#include <mitkDataNodeSelection.h>
#include <mitkErrorResponse.h>
#include <mitkIProgressService.h>
#include <mitkIRestServerService.h>
#include <mitkRestGuiThread.h>
#include <mitkRenderingManager.h>
#include <mitkUndoController.h>
#include <mitkVerboseLimitedLinearUndo.h>

#include <berryIViewDescriptor.h>
#include <berryIViewRegistry.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryPlatformUI.h>
#include <berryQtSelectionProvider.h>

#include <QItemSelectionModel>

#include <chrono>
#include <cmath>
#include <optional>

namespace
{
  const QString DataManagerViewId = "org.mitk.views.datamanager";

  /** The window the user works in, also while another application is active. */
  berry::IWorkbenchWindow::Pointer GetWindow(const std::string& path)
  {
    auto* workbench = berry::PlatformUI::GetWorkbench();
    auto window = workbench->GetActiveWorkbenchWindow();

    if (window.IsNull() && !workbench->GetWorkbenchWindows().isEmpty())
      window = workbench->GetWorkbenchWindows().front();

    if (window.IsNull() || window->GetActivePage().IsNull())
    {
      throw mitk::RestException(mitk::ErrorResponse::Create("WORKBENCH_NOT_AVAILABLE", "Service Unavailable",
        "The Workbench has no window.", 503, path));
    }

    return window;
  }

  nlohmann::json ToJson(const berry::IViewDescriptor::Pointer& view, const berry::IWorkbenchPage::Pointer& page)
  {
    const auto reference = page->FindViewReference(view->GetId());
    const auto part = reference.IsNotNull() ? reference->GetPart(false) : berry::IWorkbenchPart::Pointer();

    return {
      { "id", view->GetId().toStdString() },
      { "label", view->GetLabel().toStdString() },
      { "category", view->GetCategoryPath().join('/').toStdString() },
      { "description", view->GetDescription().toStdString() },
      { "open", reference.IsNotNull() },
      { "visible", part.IsNotNull() && page->IsPartVisible(part) }
    };
  }

  berry::IViewDescriptor::Pointer FindView(const mitk::RestRequest& request)
  {
    const auto id = QString::fromStdString(request.PathParams.at("id"));
    auto view = berry::PlatformUI::GetWorkbench()->GetViewRegistry()->Find(id);

    if (view.IsNull() || view->IsInternal())
    {
      throw mitk::RestException(mitk::ErrorResponse::Create("VIEW_NOT_FOUND", "Not Found",
        "There is no view " + id.toStdString() + ". GET /api/v1/workbench/views lists them.", 404, request.Path));
    }

    return view;
  }

  std::vector<mitk::DataNode::Pointer> GetDataManagerSelection(const std::string& path)
  {
    const auto selection = GetWindow(path)->GetSelectionService()->GetSelection(DataManagerViewId);
    const auto nodeSelection = selection.Cast<const mitk::DataNodeSelection>();

    if (nodeSelection.IsNull())
      return {};

    const auto nodes = nodeSelection->GetSelectedDataNodes();
    return { nodes.begin(), nodes.end() };
  }

  nlohmann::json ToJson(const std::vector<mitk::DataNode::Pointer>& nodes, mitk::IRestServerService* server)
  {
    auto json = nlohmann::json::array();

    for (const auto& node : nodes)
    {
      if (node.IsNotNull())
        json.push_back({ { "uid", server->GetNodeUid(node) }, { "name", node->GetName() } });
    }

    return json;
  }

  nlohmann::json DescribeHistory()
  {
    nlohmann::json history = { { "undo", nlohmann::json::array() }, { "redo", nlohmann::json::array() } };

    if (auto* model = dynamic_cast<mitk::VerboseLimitedLinearUndo*>(mitk::UndoController::GetCurrentUndoModel()))
    {
      for (const auto& [id, description] : model->GetUndoDescriptions())
        history["undo"].push_back(description);

      for (const auto& [id, description] : model->GetRedoDescriptions())
        history["redo"].push_back(description);
    }

    return history;
  }

  nlohmann::json ToJson(const mitk::ProgressTaskInfo& task)
  {
    const auto seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - task.StartTime).count();

    return {
      { "id", task.Id },
      { "name", task.Name },
      { "progress", task.StepsToDo > 0 ? nlohmann::json(static_cast<double>(task.Progress) / task.StepsToDo) : nlohmann::json() },
      { "seconds", std::round(seconds) },
      { "cancelable", task.Cancelable },
      { "cancel_requested", task.CancelRequested }
    };
  }

  std::optional<mitk::ProgressTaskInfo> FindTask(const mitk::RestRequest& request)
  {
    mitk::ProgressTaskId id = 0;

    try
    {
      id = std::stoull(request.PathParams.at("id"));
    }
    catch (const std::exception&)
    {
      throw mitk::RestException(mitk::ErrorResponse::InvalidRequest("The task id must be a number.", request.Path));
    }

    for (const auto& task : mitk::CoreServices::GetProgressService()->GetActiveTasks())
    {
      if (task.Id == id && !task.Finished)
        return task;
    }

    return std::nullopt;
  }

  /** Runs the undo or the redo of the application's Edit menu. */
  mitk::RestResponse Step(const mitk::IRestServerService& server, const mitk::RestRequest& request, bool undo)
  {
    const auto [done, history] = mitk::CallInGuiThread(server, [undo]() {
      auto* model = mitk::UndoController::GetCurrentUndoModel();
      const bool empty = model == nullptr || (undo ? model->UndoListEmpty() : model->RedoListEmpty());
      const bool stepped = !empty && (undo ? model->Undo() : model->Redo());

      if (stepped)
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      return std::make_pair(stepped, DescribeHistory());
    });

    if (!done)
    {
      return mitk::RestResponse::FromError(mitk::ErrorResponse::Create("NOTHING_TO_" + std::string(undo ? "UNDO" : "REDO"),
        "Conflict", std::string("There is nothing to ") + (undo ? "undo." : "redo."), 409, request.Path));
    }

    return mitk::RestResponse::FromData(history);
  }
}

std::vector<mitk::RestRoute> mitk::CreateWorkbenchRoutes(IRestServerService* server)
{
  std::vector<RestRoute> routes;

  RestRoute views;
  views.Method = "GET";
  views.Path = "/workbench/views";
  views.Summary = "List the views of the Workbench";
  views.Description =
    "Query parameter: open=true (optional) lists only open views. Returns {data: [{id, label, category, "
    "description, open, visible}]}; visible means the view is open and not covered by another one.";
  views.Handler = [server](const RestRequest& request) {
    const auto onlyOpen = request.GetQueryParam("open").value_or("") == "true";

    return RestResponse::FromData(CallInGuiThread(*server, [&request, onlyOpen]() {
      const auto page = GetWindow(request.Path)->GetActivePage();
      auto json = nlohmann::json::array();

      for (const auto& view : berry::PlatformUI::GetWorkbench()->GetViewRegistry()->GetViews())
      {
        if (view->IsInternal())
          continue;

        auto entry = ToJson(view, page);

        if (!onlyOpen || entry["open"].get<bool>())
          json.push_back(std::move(entry));
      }

      return json;
    }));
  };
  routes.push_back(views);

  RestRoute view;
  view.Method = "GET";
  view.Path = "/workbench/views/:id";
  view.Summary = "Get a view of the Workbench";
  view.Description = "Returns {data: {id, label, category, description, open, visible}}.";
  view.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&request]() {
      return ToJson(FindView(request), GetWindow(request.Path)->GetActivePage());
    }));
  };
  routes.push_back(view);

  RestRoute openView;
  openView.Method = "PATCH";
  openView.Path = "/workbench/views/:id";
  openView.Summary = "Open or close a view of the Workbench";
  openView.Description =
    "Body: {open: true|false}. Opening shows the view in front of the others of its tab group, without "
    "taking the keyboard focus from the user. Returns {data: view}, as GET does.";
  openView.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (!body.contains("open") || !body["open"].is_boolean())
      throw RestException(ErrorResponse::InvalidRequest("The body needs \"open\": true or false.", request.Path));

    const bool open = body["open"].get<bool>();

    return RestResponse::FromData(CallInGuiThread(*server, [&request, open]() {
      const auto descriptor = FindView(request);
      const auto page = GetWindow(request.Path)->GetActivePage();

      if (open)
      {
        page->ShowView(descriptor->GetId(), QString(), berry::IWorkbenchPage::VIEW_VISIBLE);
      }
      else if (const auto reference = page->FindViewReference(descriptor->GetId()); reference.IsNotNull())
      {
        page->HideView(reference);
      }

      return ToJson(descriptor, page);
    }));
  };
  routes.push_back(openView);

  RestRoute selection;
  selection.Method = "GET";
  selection.Path = "/workbench/selection";
  selection.Summary = "Get the data selected in the Data Manager";
  selection.Description =
    "Many views work on the data selected in the Data Manager. Returns {data: [{uid, name}]}.";
  selection.Handler = [server](const RestRequest& request) {
    const auto nodes = CallInGuiThread(*server, [&request]() { return GetDataManagerSelection(request.Path); });
    return RestResponse::FromData(ToJson(nodes, server));
  };
  routes.push_back(selection);

  RestRoute select;
  select.Method = "PUT";
  select.Path = "/workbench/selection";
  select.Summary = "Select data in the Data Manager";
  select.Description =
    "Body: {uids: [uid, ...]}; an empty list clears the selection. Needs the Data Manager to be open. Returns "
    "{data: [{uid, name}]}, as GET does.";
  select.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (!body.contains("uids") || !body["uids"].is_array())
      throw RestException(ErrorResponse::InvalidRequest("The body needs \"uids\", a list of node UIDs.", request.Path));

    std::vector<DataNode::Pointer> nodes;

    for (const auto& uid : body["uids"])
    {
      auto node = uid.is_string() ? server->FindNode(uid.get<std::string>()) : nullptr;

      if (node.IsNull())
        throw RestException(ErrorResponse::NodeNotFound(uid.is_string() ? uid.get<std::string>() : uid.dump(), request.Path));

      nodes.push_back(node);
    }

    const auto selected = CallInGuiThread(*server, [&request, &nodes]() {
      const auto dataManager = GetWindow(request.Path)->GetActivePage()->FindView(DataManagerViewId);

      if (dataManager.IsNull())
      {
        throw RestException(ErrorResponse::Create("VIEW_NOT_OPEN", "Conflict",
          "The Data Manager is not open. Open it with PATCH /api/v1/workbench/views/" +
          DataManagerViewId.toStdString() + ".", 409, request.Path));
      }

      const auto provider = dataManager->GetSite()->GetSelectionProvider().Cast<berry::QtSelectionProvider>();

      if (provider.IsNull())
        throw RestException(ErrorResponse::InternalError("The Data Manager has no selection to set.", request.Path));

      provider->SetSelection(mitk::DataNodeSelection::ConstPointer(new mitk::DataNodeSelection(nodes)), QItemSelectionModel::ClearAndSelect);

      // The Data Manager derives the "selected" properties from the
      // selection service, which lags behind when the selection is set from
      // outside, so they are set again afterwards.
      for (const auto& node : nodes)
        node->SetSelected(true);

      return GetDataManagerSelection(request.Path);
    });

    return RestResponse::FromData(ToJson(selected, server));
  };
  routes.push_back(select);

  RestRoute history;
  history.Method = "GET";
  history.Path = "/workbench/history";
  history.Summary = "List what undo and redo would revert or repeat";
  history.Description =
    "The history of the application's Edit menu, which covers changes of segmentations, but not adding or "
    "removing data. Returns {data: {undo: [description], redo: [description]}}, most recent first.";
  history.Handler = [server](const RestRequest&) {
    return RestResponse::FromData(CallInGuiThread(*server, []() { return DescribeHistory(); }));
  };
  routes.push_back(history);

  RestRoute undo;
  undo.Method = "POST";
  undo.Path = "/workbench/undo";
  undo.Summary = "Undo the last change, as Edit > Undo does";
  undo.Description = "Returns {data: {undo, redo}} as GET /api/v1/workbench/history does, or 409 if there is nothing to undo.";
  undo.Handler = [server](const RestRequest& request) { return Step(*server, request, true); };
  routes.push_back(undo);

  RestRoute redo;
  redo.Method = "POST";
  redo.Path = "/workbench/redo";
  redo.Summary = "Redo the last undone change, as Edit > Redo does";
  redo.Description = "Returns {data: {undo, redo}} as GET /api/v1/workbench/history does, or 409 if there is nothing to redo.";
  redo.Handler = [server](const RestRequest& request) { return Step(*server, request, false); };
  routes.push_back(redo);

  RestRoute tasks;
  tasks.Method = "GET";
  tasks.Path = "/workbench/tasks";
  tasks.Summary = "List the tasks that run, as the progress notifications show them";
  tasks.Description =
    "Downloads, loading models and computations, whether the user started them or a request did. Returns "
    "{data: [{id, name, progress (0 to 1, or null if unknown), seconds, cancelable, cancel_requested}]}. While "
    "a task runs, the Workbench may answer slowly; wait for it before relying on its results.";
  tasks.Handler = [](const RestRequest&) {
    auto json = nlohmann::json::array();

    for (const auto& task : CoreServices::GetProgressService()->GetActiveTasks())
    {
      if (!task.Finished)
        json.push_back(ToJson(task));
    }

    return RestResponse::FromData(json);
  };
  routes.push_back(tasks);

  RestRoute cancelTask;
  cancelTask.Method = "DELETE";
  cancelTask.Path = "/workbench/tasks/:id";
  cancelTask.Summary = "Cancel a task, as the cancel button of its progress notification does";
  cancelTask.Description = "Returns {data: task}, or 409 if the task cannot be cancelled.";
  cancelTask.Handler = [](const RestRequest& request) {
    const auto task = FindTask(request);

    if (!task.has_value())
    {
      throw RestException(ErrorResponse::Create("TASK_NOT_FOUND", "Not Found",
        "There is no running task " + request.PathParams.at("id") + ".", 404, request.Path));
    }

    if (!task->Cancelable)
      throw RestException(ErrorResponse::Create("TASK_NOT_CANCELABLE", "Conflict", "The task cannot be cancelled.", 409, request.Path));

    CoreServices::GetProgressService()->RequestCancel(task->Id);

    auto json = ToJson(*task);
    json["cancel_requested"] = true;
    return RestResponse::FromData(json);
  };
  routes.push_back(cancelTask);

  return routes;
}
