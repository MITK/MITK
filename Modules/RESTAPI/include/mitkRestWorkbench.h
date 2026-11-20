/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestWorkbench_h
#define mitkRestWorkbench_h

#include <MitkRESTAPIExports.h>

#include <mitkIProgressListener.h>
#include <mitkRestRoute.h>

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

namespace mitk
{
  class IProgressService;
  class UndoModel;
  class VerboseLimitedLinearUndo;

  /**
   * \brief The history of the application's Edit menu as
   *        {undo: [{description, own}], redo: [{description, own}]}, most
   *        recent first.
   *
   * own tells whether the step was made under the ActionOriginScope that is
   * current: call where the client's origin scope holds.
   *
   * \param model The undo model; without one, or one that keeps no
   *        descriptions, both lists are empty.
   */
  MITKRESTAPI_EXPORT nlohmann::json DescribeRestHistory(VerboseLimitedLinearUndo* model);

  /**
   * \brief The description of the step that an undo or redo would take, as the
   *        history lists it first; empty if there is none.
   */
  MITKRESTAPI_EXPORT std::string DescribeRestNextStep(VerboseLimitedLinearUndo* model, bool undo);

  /**
   * \brief Runs the undo or the redo of the model, if the step on top is made
   *        under the ActionOriginScope that is current.
   *
   * Call where the client's origin scope holds, in the GUI thread.
   *
   * \param model The undo model; may be null.
   * \return Status 200 with {undone|redone, undo, redo}, the history as
   *         DescribeRestHistory() lists it afterwards; 409 NOTHING_TO_UNDO or
   *         NOTHING_TO_REDO if there is no step, and 409 NOT_YOUR_STEP if the
   *         step is not the client's own, or no origins are known to tell.
   */
  MITKRESTAPI_EXPORT RestResponse TakeRestStep(UndoModel* model, bool undo, const std::string& path);

  /**
   * \brief A task as GET /api/v1/workbench/tasks describes it: {id, name,
   *        progress (0 to 1, or null if unknown), seconds, own, cancelable,
   *        cancel_requested}.
   */
  MITKRESTAPI_EXPORT nlohmann::json ToRestJson(const ProgressTaskInfo& task);

  /**
   * \brief The running task that a path parameter names.
   *
   * \param id The text of the path parameter.
   * \return No value if no unfinished task has the id.
   * \throw RestException with status 400 unless the whole id is a number.
   */
  MITKRESTAPI_EXPORT std::optional<ProgressTaskInfo> FindRestTask(
    const IProgressService& service, const std::string& id, const std::string& path);
}

#endif
