/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryClosePerspectiveHandler.h"

#include <berryHandlerUtil.h>

#include "internal/berryPerspective.h"
#include "internal/berryWorkbenchPage.h"

#include <berryIWorkbench.h>
#include <berryIWorkbenchCommandConstants.h>

namespace berry {

Object::Pointer ClosePerspectiveHandler::Execute(const SmartPointer<const ExecutionEvent>& event)
{
  IWorkbenchWindow::Pointer activeWorkbenchWindow = HandlerUtil::GetActiveWorkbenchWindow(event);
  if (activeWorkbenchWindow.IsNotNull())
  {
    WorkbenchPage::Pointer page = activeWorkbenchWindow->GetActivePage().Cast<WorkbenchPage>();
    if (page.IsNotNull())
    {
      const ExecutionEvent::ParameterMap& parameters = event->GetParameters();
      QString value = parameters[IWorkbenchCommandConstants::WINDOW_CLOSE_PERSPECTIVE_PARM_ID];
      if (value.isEmpty())
      {
        page->ClosePerspective(page->GetPerspective(), true, true);
      }
      else
      {
        IPerspectiveDescriptor::Pointer perspective = activeWorkbenchWindow
                                             ->GetWorkbench()->GetPerspectiveRegistry()
                                             ->FindPerspectiveWithId(value);
        if (perspective.IsNotNull())
        {
          page->ClosePerspective(perspective, true, true);
        }
      }
    }
  }
  return Object::Pointer();
}

void ClosePerspectiveHandler::ClosePerspective(const SmartPointer<WorkbenchPage>& page, const SmartPointer<Perspective>& persp)
{
  if (page != nullptr && persp != nullptr)
  {
    page->ClosePerspective(persp->GetDesc(), true, true);
  }
}

}
