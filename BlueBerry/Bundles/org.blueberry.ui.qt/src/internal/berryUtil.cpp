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

#include "berryUtil.h"

#include "berryShell.h"
#include "berryPlatformUI.h"

namespace berry {

SmartPointer<Shell> Util::GetShellToParentOn()
{
  IWorkbench* workbench = PlatformUI::GetWorkbench();
  IWorkbenchWindow::Pointer activeWindow = workbench->GetActiveWorkbenchWindow();
  IWorkbenchWindow::Pointer windowToParentOn = activeWindow.IsNull() ?
        (workbench->GetWorkbenchWindowCount() > 0 ?
           workbench->GetWorkbenchWindows()[0] : IWorkbenchWindow::Pointer(0))
      : activeWindow;
  return windowToParentOn.IsNull() ? Shell::Pointer(0) : activeWindow->GetShell();
}

}
