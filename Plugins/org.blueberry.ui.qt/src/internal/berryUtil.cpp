/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
           workbench->GetWorkbenchWindows()[0] : IWorkbenchWindow::Pointer(nullptr))
      : activeWindow;
  return windowToParentOn.IsNull() ? Shell::Pointer(nullptr) : activeWindow->GetShell();
}

}
