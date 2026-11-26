/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestPythonSegmentation_h
#define mitkRestPythonSegmentation_h

#include <MitkSegmentationRESTExports.h>

#include <mitkRestJob.h>
#include <mitkTotalSegmentatorTool.h>
#include <mitkVoxTellTool.h>
#include <mitknnInteractiveTool.h>

#include <nlohmann/json.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <string>

namespace mitk
{
  class ToolManager;

  /**
   * \brief The state of an nnInteractive session as clients read it:
   *        {session_running, session_starting, remote, prompt_types,
   *        can_undo, auto_zoom, auto_refine, preview_voxels}.
   *
   * preview_voxels is 0 without a running session.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestNNInteractive(nnInteractiveTool* tool);

  /**
   * \brief The interaction type that the field "type" of a request body names:
   *        point, box, scribble or lasso.
   *
   * \param path The request path, for errors.
   * \throw RestException with status 400 for a missing, wrong-typed or other value.
   */
  MITKSEGMENTATIONREST_EXPORT nnInteractive::InteractionType ParseRestInteractionType(
    const nlohmann::json& body, const std::string& path);

  /**
   * \brief Ends the job of a start with the state of the tool once the panel
   *        has seeded the session, which it does on the event that the tool
   *        sends after its start has ended.
   *
   * Queues the call to the event loop of the application, so that it runs
   * after that event. Waits for any work of the tool, also a prompt, and
   * fails the job if the session was lost meanwhile (a cancel by the user in
   * the words "canceled in the Workbench") or ended.
   */
  MITKSEGMENTATIONREST_EXPORT void SucceedRestJobWhenSessionSeeded(
    const std::shared_ptr<RestJob>& job, const nnInteractiveTool::Pointer& tool);

  /**
   * \brief The state of VoxTell as clients read it: {installed, model_loaded,
   *        model_loading, prompts, prompts_without_result, error,
   *        preview_labels}.
   *
   * \param installed Whether VoxTell has its Python environment.
   * \param measure Whether the preview labels have voxels, volume_ml and
   *                center, which take a pass over them.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestVoxTell(
    VoxTellTool* tool, ToolManager* toolManager, bool installed, bool measure = false);

  /** \brief A speed as clients write it: standard, fast or fastest. */
  MITKSEGMENTATIONREST_EXPORT std::string ToRestJson(TotalSegmentatorTool::Speed speed);

  /**
   * \brief The speed of a JSON value, which must be one of the three names.
   *
   * \param path The request path, for errors.
   * \throw RestException with status 400 for any other value, also a non-string.
   */
  MITKSEGMENTATIONREST_EXPORT TotalSegmentatorTool::Speed ParseRestSpeed(const nlohmann::json& json, const std::string& path);

  /**
   * \brief A TotalSegmentator task as clients read it: {name, modality,
   *        licensed, available, speeds, roi_subset, class_count}, with
   *        classes if requested, which can be many.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestTotalSegmentatorTask(
    const TotalSegmentatorTool::TaskInfo& task, bool withClasses);

  /**
   * \brief The state of TotalSegmentator as clients read it.
   *
   * \param withTasks Whether to list the tasks, once the tool is initialized.
   * \param measure Whether the preview labels have voxels, volume_ml and
   *                center, which take a pass over them.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestTotalSegmentator(
    TotalSegmentatorTool* tool, ToolManager* toolManager, bool withTasks, bool measure = false);

  /**
   * \brief Starts TotalSegmentator unless it runs or starts already, then
   *        calls \p done on the GUI thread with null, or with why it does
   *        not start.
   *
   * Waits until the tool has started, or failed to, whoever started it, as
   * the tool tells that only by events. \p done runs under the
   * mitk::ActionOriginScope of the caller.
   */
  MITKSEGMENTATIONREST_EXPORT void StartRestTotalSegmentator(
    TotalSegmentatorTool* tool, std::function<void(std::exception_ptr)> done);
}

#endif
