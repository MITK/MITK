/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkInitializationHook.h"

#include <mitkExceptionMacro.h>
#include <mitkLog.h>
#include <mitkProgressTask.h>
#include <mitkTool.h>

mitk::PythonWorker::ProgressHook mitk::MakeInitializationHook(Tool* tool, const ProgressTask* task, std::function<bool()> abort)
{
  return [tool, task, abort = std::move(abort)](const nlohmann::json&)
  {
    try
    {
      tool->GUIProcessEventsMessage.Send();
    }
    catch (const std::exception& e)
    {
      MITK_WARN << tool->GetName() << ": error while processing events (ignored): " << e.what();
    }

    if (task != nullptr && task->IsCancelRequested())
      mitkThrow() << tool->GetName() << ": the initialization was cancelled.";

    if (abort && abort())
      mitkThrow() << tool->GetName() << ": the initialization was aborted.";

    return true;
  };
}
