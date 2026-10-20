/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInitializationHook_h
#define mitkInitializationHook_h

#include <mitkPythonWorker.h>

#include <functional>

namespace mitk
{
  class ProgressTask;
  class Tool;

  /**
   * \brief Returns a progress hook for the worker calls that initialize a tool.
   *
   * The hook processes events through the GUIProcessEventsMessage of the tool,
   * which keeps the application responsive while a model is downloaded or
   * loaded. Once \p task is cancelled or \p abort returns true, it throws, which
   * makes the worker end right away: these calls do not check for a
   * cancellation themselves, and a download would otherwise run on.
   *
   * Events may deactivate the tool, so the tool has to defer its teardown
   * while it initializes and let \p abort report it.
   *
   * \param[in] tool The tool whose events are processed.
   * \param[in] task The task the user can cancel, or \c nullptr.
   * \param[in] abort Tells whether the tool was deactivated meanwhile.
   */
  PythonWorker::ProgressHook MakeInitializationHook(Tool* tool, const ProgressTask* task, std::function<bool()> abort);
}

#endif
