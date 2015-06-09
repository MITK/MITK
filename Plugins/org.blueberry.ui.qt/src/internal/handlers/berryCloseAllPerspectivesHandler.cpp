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
