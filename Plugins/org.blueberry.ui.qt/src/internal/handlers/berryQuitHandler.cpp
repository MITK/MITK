/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQuitHandler.h"

#include <berryIWorkbench.h>
#include <berryHandlerUtil.h>

namespace berry {

Object::Pointer QuitHandler::Execute(const SmartPointer<const ExecutionEvent>& event)
{
  IWorkbenchWindow::Pointer activeWorkbenchWindow = HandlerUtil::GetActiveWorkbenchWindow(event);
  if (activeWorkbenchWindow.IsNull())
  {
    // action has been disposed
    return Object::Pointer();
  }

  activeWorkbenchWindow->GetWorkbench()->Close();
  return Object::Pointer();
}

}
