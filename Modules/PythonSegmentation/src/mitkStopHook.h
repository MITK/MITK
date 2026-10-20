/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStopHook_h
#define mitkStopHook_h

#include <mitkPythonWorker.h>

#include <functional>

namespace mitk
{
  class ProgressTask;
  class Tool;

  /**
   * \brief Returns a progress hook that ends a worker call once it is to stop.
   *
   * Once \p task is cancelled or \p abort returns true, the hook throws, which
   * makes the worker end right away: calls like loading a model or running a
   * network do not check for a cancellation themselves, and a download would
   * otherwise run on.
   *
   * The hook runs on the thread of the call, off the GUI thread, so \p abort
   * must be safe to call from there.
   *
   * \param[in] tool The tool whose name the exception gives.
   * \param[in] task The task the user can cancel, or \c nullptr.
   * \param[in] abort Tells whether the work is to stop for another reason than
   *            a cancellation by the user, like a deactivation of the tool or
   *            an aborted session.
   */
  PythonWorker::ProgressHook MakeStopHook(const Tool* tool, const ProgressTask* task, std::function<bool()> abort);
}

#endif
