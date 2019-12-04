/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCloseAllPerspectivesHandler.h"

#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryHandlerUtil.h>

namespace berry {

Object::Pointer CloseAllPerspectivesHandler::Execute(const SmartPointer<const ExecutionEvent>& event)
{
  IWorkbenchWindow::Pointer window = HandlerUtil::GetActiveWorkbenchWindow(event);
  if (window.IsNotNull())
  {
    IWorkbenchPage::Pointer page = window->GetActivePage();
    if (page.IsNotNull())
    {
      page->CloseAllPerspectives(true, true);
    }
  }
  return Object::Pointer();
}

}
