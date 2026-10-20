/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkStopHook.h"

#include <mitkExceptionMacro.h>
#include <mitkProgressTask.h>
#include <mitkTool.h>

mitk::PythonWorker::ProgressHook mitk::MakeStopHook(const Tool* tool, const ProgressTask* task, std::function<bool()> abort)
{
  return [tool, task, abort = std::move(abort)](const nlohmann::json&)
  {
    if (task != nullptr && task->IsCancelRequested())
      mitkThrow() << tool->GetName() << ": cancelled.";

    if (abort && abort())
      mitkThrow() << tool->GetName() << ": aborted.";

    return true;
  };
}
