/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySavePerspectiveHandler.h"

#include "berryHandlerUtil.h"
#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"

#include "internal/berryPerspectiveDescriptor.h"
#include "berryWorkbenchPlugin.h"
#include "internal/dialogs/berrySavePerspectiveDialog.h"

#include <QMessageBox>

#include <cassert>

namespace berry {

Object::Pointer SavePerspectiveHandler::Execute(const SmartPointer<const berry::ExecutionEvent>& event)
{

  IWorkbenchWindow::Pointer activeWorkbenchWindow = HandlerUtil::GetActiveWorkbenchWindow(event);
  if (activeWorkbenchWindow.IsNotNull())
  {
    IWorkbenchPage::Pointer page = activeWorkbenchWindow->GetActivePage();
    if (page.IsNotNull())
    {
      PerspectiveDescriptor::Pointer descriptor(page->GetPerspective().Cast<PerspectiveDescriptor>());
      if (descriptor.IsNotNull())
      {
        if (descriptor->IsSingleton())
        {
          SaveSingleton(page);
        }
        else
        {
          SaveNonSingleton(page, descriptor);
        }
      }
    }
  }
  return Object::Pointer();
}

void SavePerspectiveHandler::SaveSingleton(const IWorkbenchPage::Pointer& page)
{
  QMessageBox::StandardButton btn =
      QMessageBox::question(page->GetWorkbenchWindow()->GetShell()->GetControl(),
                            "Overwrite perspective",
                            "The current perspective can only be opened once and cannot be saved using a new name. "
                            "Do you want to overwrite?",
                            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
  if (btn == QMessageBox::Ok)
  {
    page->SavePerspective();
  }
}

void SavePerspectiveHandler::SaveNonSingleton(const IWorkbenchPage::Pointer& page,
                                              const PerspectiveDescriptor::Pointer& oldDesc) {
  // Get reg.
  PerspectiveRegistry* reg = static_cast<PerspectiveRegistry*>(
                               berry::WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry());
  assert(reg != nullptr);

  // Get persp name.
  SavePerspectiveDialog dlg(*reg, page->GetWorkbenchWindow()->GetShell()->GetControl());
  // Look up the descriptor by id again to ensure it is still valid.
  IPerspectiveDescriptor::Pointer description = reg->FindPerspectiveWithId(oldDesc->GetId());
  dlg.SetInitialSelection(description);
  if (dlg.exec() != QDialog::Accepted)
  {
    return;
  }

  // Create descriptor.
  IPerspectiveDescriptor::Pointer newDesc = dlg.GetPersp();
  if (newDesc.IsNull())
  {
    QString name = dlg.GetPerspName();
    newDesc = reg->CreatePerspective(name, description);
    if (newDesc.IsNull())
    {
      QMessageBox::critical(&dlg,
                            "Cannot Save Perspective",
                            "Invalid Perspective Descriptor");
      return;
    }
  }

  // Save state.
  page->SavePerspectiveAs(newDesc);
}

}
