/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryResetPerspectiveHandler.h"

#include <berryIWorkbenchWindow.h>
#include <berryHandlerUtil.h>

#include <internal/berryWorkbenchPage.h>
#include <internal/berryPerspectiveDescriptor.h>

#include <QMessageBox>

namespace berry {

Object::Pointer ResetPerspectiveHandler::Execute(const SmartPointer<const ExecutionEvent>& event)
{

  IWorkbenchWindow::Pointer activeWorkbenchWindow = HandlerUtil::GetActiveWorkbenchWindow(event);
  if (activeWorkbenchWindow.IsNotNull())
  {
    WorkbenchPage::Pointer page = activeWorkbenchWindow->GetActivePage().Cast<WorkbenchPage>();
    if (page.IsNotNull())
    {
      IPerspectiveDescriptor::Pointer descriptor = page->GetPerspective();
      if (descriptor.IsNotNull())
      {
        bool offerRevertToBase = false;
        if (PerspectiveDescriptor::Pointer desc = descriptor.Cast<PerspectiveDescriptor>())
        {
          offerRevertToBase = desc->IsPredefined() && desc->HasCustomDefinition();
        }

        if (offerRevertToBase)
        {
          QString message = QString("Do you want to reset the current %1 perspective to its saved state?").arg(
                              descriptor->GetLabel());
//          bool toggleState = false;
//          MessageDialogWithToggle dialog = MessageDialogWithToggle.open(
//                                             MessageDialog.QUESTION, activeWorkbenchWindow.getShell(),
//                                             WorkbenchMessages.RevertPerspective_title, message,
//                                             WorkbenchMessages.RevertPerspective_option,
//                                             toggleState, null, null, SWT.SHEET);
//          if (dialog.getReturnCode() == IDialogConstants.YES_ID) {
//            if (dialog.getToggleState()) {
//              PerspectiveRegistry* reg = PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
//              reg->RevertPerspective(descriptor);
//            }
//            page->ResetPerspective();
//          }
          qDebug() << "****** " << message;
        }
        else
        {
          QString message = QString("Do you want to reset the current %1 perspective to its defaults?").arg(
                                    descriptor->GetLabel());
          if (QMessageBox::question(activeWorkbenchWindow->GetShell()->GetControl(), "Reset Perspective",
                                    message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ==
              QMessageBox::Yes)
          {
            page->ResetPerspective();
          }
        }
      }
    }
  }

  return Object::Pointer();
}


}
