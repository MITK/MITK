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

#include "berryOpenInNewWindowHandler.h"

#include <berryHandlerUtil.h>

#include <berryIAdaptable.h>
#include <berryIPerspectiveRegistry.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

#include <berryUIException.h>

#include "internal/berryWorkbench.h"

#include <QApplication>
#include <QMessageBox>

namespace berry {

Object::Pointer OpenInNewWindowHandler::Execute(const SmartPointer<const ExecutionEvent> &event)
{
  IWorkbenchWindow::Pointer activeWorkbenchWindow = HandlerUtil::GetActiveWorkbenchWindow(event);
  if (activeWorkbenchWindow.IsNull())
  {
    return Object::Pointer();
  }

  try
  {
    QString perspId;

    IWorkbenchPage::Pointer page = activeWorkbenchWindow->GetActivePage();
    IAdaptable* pageInput = dynamic_cast<Workbench*>(activeWorkbenchWindow->GetWorkbench())->GetDefaultPageInput();
    if (page.IsNotNull() && page->GetPerspective().IsNotNull())
    {
      perspId = page->GetPerspective()->GetId();
      pageInput = page->GetInput();
    }
    else
    {
      perspId = activeWorkbenchWindow->GetWorkbench()
          ->GetPerspectiveRegistry()->GetDefaultPerspective();
    }

    activeWorkbenchWindow->GetWorkbench()->OpenWorkbenchWindow(perspId,
                                                               pageInput);
  }
  catch (const WorkbenchException& e)
  {
//    StatusUtil.handleStatus(e.getStatus(),
//                            "Problems Opening New Window: " + e.message(),
//                            StatusManager::SHOW);
    QMessageBox::critical(QApplication::activeWindow(), "Problems Opening New Window", e.message());
  }
  return Object::Pointer();
}

}
