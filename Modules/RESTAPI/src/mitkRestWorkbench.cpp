/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestWorkbench.h>

#include <mitkActionOriginScope.h>
#include <mitkErrorResponse.h>
#include <mitkIProgressService.h>
#include <mitkRenderingManager.h>
#include <mitkUndoModel.h>
#include <mitkVerboseLimitedLinearUndo.h>

#include <charconv>
#include <chrono>
#include <cmath>
#include <vector>

namespace
{
  /** The entries of one stack, each with whether the asking client made it. Call where the client's origin scope holds. */
  nlohmann::json DescribeEntries(const mitk::VerboseLimitedLinearUndo::StackDescription& descriptions,
                                 const std::vector<std::string>& origins)
  {
    const auto client = mitk::ActionOriginScope::GetCurrent();
    auto entries = nlohmann::json::array();

    for (std::size_t i = 0; i < descriptions.size(); ++i)
      entries.push_back({ { "description", descriptions[i].second }, { "own", origins[i] == client } });

    return entries;
  }

  struct StepOutcome
  {
    enum class State
    {
      NothingToDo,
      NotOwn,
      Done
    };

    State Result = State::NothingToDo;

    /** The description of the step that was taken, or of the one that is not the client's. */
    std::string Step;

    nlohmann::json History;
  };

  StepOutcome Take(mitk::UndoModel* model, bool undo)
  {
    StepOutcome taken;

    if (model == nullptr || (undo ? model->UndoListEmpty() : model->RedoListEmpty()))
      return taken;

    auto* historyModel = dynamic_cast<mitk::VerboseLimitedLinearUndo*>(model);
    taken.Step = mitk::DescribeRestNextStep(historyModel, undo);

    // Without origins nothing can be told to be the client's own.
    const auto origins = historyModel != nullptr
      ? (undo ? historyModel->GetUndoOrigins() : historyModel->GetRedoOrigins())
      : std::vector<std::string>();

    if (origins.empty() || origins.front() != mitk::ActionOriginScope::GetCurrent())
    {
      taken.Result = StepOutcome::State::NotOwn;
      return taken;
    }

    if (!(undo ? model->Undo() : model->Redo()))
      return taken;

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    taken.Result = StepOutcome::State::Done;
    taken.History = mitk::DescribeRestHistory(historyModel);
    taken.History[undo ? "undone" : "redone"] = taken.Step;

    return taken;
  }
}

nlohmann::json mitk::DescribeRestHistory(VerboseLimitedLinearUndo* model)
{
  nlohmann::json history = { { "undo", nlohmann::json::array() }, { "redo", nlohmann::json::array() } };

  if (model != nullptr)
  {
    history["undo"] = DescribeEntries(model->GetUndoDescriptions(), model->GetUndoOrigins());
    history["redo"] = DescribeEntries(model->GetRedoDescriptions(), model->GetRedoOrigins());
  }

  return history;
}

std::string mitk::DescribeRestNextStep(VerboseLimitedLinearUndo* model, bool undo)
{
  if (model != nullptr)
  {
    const auto descriptions = undo ? model->GetUndoDescriptions() : model->GetRedoDescriptions();

    if (!descriptions.empty())
      return descriptions.front().second;
  }

  return std::string();
}

mitk::RestResponse mitk::TakeRestStep(UndoModel* model, bool undo, const std::string& path)
{
  const auto outcome = Take(model, undo);

  switch (outcome.Result)
  {
    case StepOutcome::State::NotOwn:
      return RestResponse::FromError(ErrorResponse::Create("NOT_YOUR_STEP", "Conflict",
        "The step on top of the history, \"" + outcome.Step + "\", is not yours. Ask the user to " +
          (undo ? "undo" : "redo") + " it.",
        409, path));

    case StepOutcome::State::NothingToDo:
      return RestResponse::FromError(ErrorResponse::Create("NOTHING_TO_" + std::string(undo ? "UNDO" : "REDO"),
        "Conflict", std::string("There is nothing to ") + (undo ? "undo." : "redo."), 409, path));

    case StepOutcome::State::Done:
      break;
  }

  return RestResponse::FromData(outcome.History);
}

nlohmann::json mitk::ToRestJson(const ProgressTaskInfo& task)
{
  const auto seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - task.StartTime).count();

  return {
    { "id", task.Id },
    { "name", task.Name },
    { "progress", task.StepsToDo > 0 ? nlohmann::json(static_cast<double>(task.Progress) / task.StepsToDo) : nlohmann::json() },
    { "seconds", std::round(seconds) },
    { "own", task.Origin == ActionOriginScope::GetCurrent() },
    { "cancelable", task.Cancelable },
    { "cancel_requested", task.CancelRequested }
  };
}

std::optional<mitk::ProgressTaskInfo> mitk::FindRestTask(
  const IProgressService& service, const std::string& text, const std::string& path)
{
  const auto* textEnd = text.data() + text.size();
  ProgressTaskId id = 0;

  if (const auto [end, error] = std::from_chars(text.data(), textEnd, id); error != std::errc() || end != textEnd)
    throw RestException(ErrorResponse::InvalidRequest("The task id must be a number.", path));

  for (const auto& task : service.GetActiveTasks())
  {
    if (task.Id == id && !task.Finished)
      return task;
  }

  return std::nullopt;
}
