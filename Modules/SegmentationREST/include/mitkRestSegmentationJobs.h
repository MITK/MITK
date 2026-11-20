/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestSegmentationJobs_h
#define mitkRestSegmentationJobs_h

#include <MitkSegmentationRESTExports.h>

#include <mitkRestJob.h>
#include <mitkSegWithPreviewTool.h>

#include <nlohmann/json.hpp>

#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <string>

namespace mitk
{
  class Tool;
  class ToolManager;

  /**
   * \brief Ends a job with what an operation threw: the message of a
   *        RestException, the description of an mitk::Exception, or the
   *        message of any other exception.
   *
   * \param error The exception to rethrow; must not be null.
   */
  MITKSEGMENTATIONREST_EXPORT void FailRestJob(RestJob& job, std::exception_ptr error);

  /** \brief An mitk::Exception with the given message, for a callback that passes errors on. */
  MITKSEGMENTATIONREST_EXPORT std::exception_ptr MakeRestError(const std::string& message);

  /**
   * \brief Runs \p then once the tool neither updates nor runs other work
   *        off the GUI thread: at once if it is idle, otherwise from the
   *        event loop of the application.
   *
   * Polls rather than listening to the end of the update, because a
   * confirmation that follows an update transfers only after the tool has
   * reported that end. Runs in the GUI thread.
   */
  MITKSEGMENTATIONREST_EXPORT void WhenRestToolIdle(const SegWithPreviewTool::Pointer& tool, std::function<void()> then);

  /**
   * \brief Why the last confirmation of the tool did not transfer, or empty
   *        if it did.
   *
   * Without a confirmation that has ended, it says so, so that an empty
   * answer always means that the result was confirmed.
   */
  MITKSEGMENTATIONREST_EXPORT std::string GetRestConfirmationFailure(const SegWithPreviewTool& tool);

  /**
   * \brief Refuses a request while the tool works, which takes one thing at a time.
   *
   * \param path The request path, for the error.
   * \throw RestException with status 409 and the code BUSY.
   */
  MITKSEGMENTATIONREST_EXPORT void ThrowIfRestToolBusy(const SegWithPreviewTool* tool, const std::string& path);

  /**
   * \brief Runs \p then once \p isStarting is false, for a start that the
   *        panel of a tool began.
   *
   * Polls as WhenRestToolIdle() does. Gives up, without calling \p then,
   * when the job has ended meanwhile.
   */
  MITKSEGMENTATIONREST_EXPORT void WhenRestStartEnded(
    const std::shared_ptr<RestJob>& job, std::function<bool()> isStarting, std::function<void()> then);

  /**
   * \brief Fails the job if the tool is no longer the active tool of the manager.
   *
   * A deactivation ends the work of a tool without a result, and some tools
   * do not record that as a failure, so their state would read as a run that
   * found nothing.
   *
   * \return Whether the job was failed.
   */
  MITKSEGMENTATIONREST_EXPORT bool FailRestJobIfDeactivated(RestJob& job, ToolManager* toolManager, const Tool* tool);

  /**
   * \brief The labels of the preview of a tool that adds labels, as the
   *        confirm adds them: without their values, which only the confirm
   *        gives them, and in the group of the active label of the working
   *        segmentation, unless the tool transfers all groups of its
   *        preview, which then stand for those of the segmentation.
   *
   * \param measure Whether the labels have voxels, volume_ml and center at
   *                the selected time point.
   * \return An empty array while the tool has no preview.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestPreviewLabels(
    SegWithPreviewTool* tool, ToolManager* toolManager, bool measure);

  /**
   * \brief The voxels of all labels of the preview at the selected time point.
   *
   * A group that a tool writes into is left out: the count never waits for
   * the tool. 0 without a preview or without a time step at the time point.
   */
  MITKSEGMENTATIONREST_EXPORT std::uint64_t CountRestPreviewVoxels(SegWithPreviewTool* tool);

  /**
   * \brief How a prompt ended and the job that reports it.
   *
   * The job is made after the tool took the prompt, which can end before
   * that, so whichever of the two comes second passes the outcome on. Used
   * in the GUI thread only.
   */
  struct MITKSEGMENTATIONREST_EXPORT RestPromptOutcome
  {
    std::shared_ptr<RestJob> Job;
    bool Ended = false;
    std::exception_ptr Error;
    nlohmann::json Result;

    /** Ends the job with the error or the result once there is a job and the prompt has ended; else nothing. */
    void ReportToJob() const;
  };
}

#endif
