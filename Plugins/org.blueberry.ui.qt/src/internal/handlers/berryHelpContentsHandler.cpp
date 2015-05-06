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
