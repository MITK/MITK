/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
