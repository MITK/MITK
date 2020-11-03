/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHelpContentsHandler.h"

#include <berryHandlerUtil.h>
#include <berryPlatformUI.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>


namespace berry {

Object::Pointer HelpContentsHandler::Execute(const SmartPointer<const ExecutionEvent>& event)
{
  IWorkbenchWindow::Pointer window = HandlerUtil::GetActiveWorkbenchWindow(event);
  PlatformUI::GetWorkbench()->ShowPerspective("org.blueberry.perspectives.help",
                                              window);
  return Object::Pointer();
}

bool HelpContentsHandler::IsEnabled() const
{
  return PlatformUI::GetWorkbench()->GetPerspectiveRegistry()
      ->FindPerspectiveWithId("org.blueberry.perspectives.help").IsNotNull();
}

}
