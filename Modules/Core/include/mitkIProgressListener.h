/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIProgressListener_h
#define mitkIProgressListener_h

#include <mitkProgressTaskInfo.h>

namespace mitk
{
  /**
   * \brief Interface for observing the tasks of the IProgressService.
   *
   * To receive snapshots, listeners must first subscribe via
   * IProgressService::AddListener().
   *
   * \sa IProgressService
   */
  class MITKCORE_EXPORT IProgressListener
  {
  public:
    virtual ~IProgressListener();

    /**
     * \brief Callback for a new snapshot of a task.
     *
     * Called from whichever thread reported the progress, which is frequently
     * not the GUI thread. Implementations must neither block nor report
     * progress themselves: the service serializes listener notification per
     * registration, so doing either stalls every reporting thread.
     *
     * Snapshots can arrive out of order and after a task has finished. Apply
     * them as a reduction keyed by ProgressTaskInfo::Id and discard anything
     * whose ProgressTaskInfo::Sequence is not newer than the last applied one.
     *
     * \param info A complete snapshot of one task.
     */
    virtual void OnTaskUpdated(const ProgressTaskInfo& info) = 0;
  };
}

#endif
