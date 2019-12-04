/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySaveable.h"

#include "berryIWorkbenchPage.h"
#include "berryIWorkbenchPart.h"

namespace berry
{

bool Saveable::Show(IWorkbenchPage::Pointer  /*page*/)
{
  return false;
}

/*IJobRunnable*/void Saveable::DoSave(/*IProgressMonitor monitor,
 IShellProvider shellProvider*/)
{
}

void Saveable::DisableUI(const QList<IWorkbenchPart::Pointer>& /*parts*/,
    bool  /*closing*/)
{
  //TODO Saveable DisableUI
  //    for (int i = 0; i < parts.length; i++) {
  //      IWorkbenchPart workbenchPart = parts[i];
  //      Composite paneComposite = (Composite) ((PartSite) workbenchPart
  //          .getSite()).getPane().getControl();
  //      Control[] paneChildren = paneComposite.getChildren();
  //      Composite toDisable = ((Composite) paneChildren[0]);
  //      toDisable.setEnabled(false);
  //      if (waitCursor == null) {
  //        waitCursor = new Cursor(workbenchPart.getSite().getWorkbenchWindow().getShell().getDisplay(), SWT.CURSOR_WAIT);
  //      }
  //      originalCursor = paneComposite.getCursor();
  //      paneComposite.setCursor(waitCursor);
  //    }
}

void Saveable::EnableUI(QList<IWorkbenchPart::Pointer>&  /*parts*/)
{
  //TODO Saveable EnableUI
  //    for (unsigned int i = 0; i < parts.size(); i++) {
  //      IWorkbenchPart::Pointer workbenchPart = parts[i];
  //      Composite paneComposite = (Composite) ((PartSite) workbenchPart
  //          .getSite()).getPane().getControl();
  //      Control[] paneChildren = paneComposite.getChildren();
  //      Composite toEnable = ((Composite) paneChildren[0]);
  //      paneComposite.setCursor(originalCursor);
  //      if (waitCursor!=null && !waitCursor.isDisposed()) {
  //        waitCursor.dispose();
  //        waitCursor = null;
  //      }
  //      toEnable.setEnabled(true);
  //    }
}

Object* Saveable::GetAdapter(const QString&  /*adapter*/) const
{
  return nullptr;
}

}
