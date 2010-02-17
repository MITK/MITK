/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

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
  this->DoSave(/*monitor*/);
  //return 0;
}

void Saveable::DisableUI(const std::vector<IWorkbenchPart::Pointer>& /*parts*/,
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

void Saveable::EnableUI(std::vector<IWorkbenchPart::Pointer>&  /*parts*/)
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

Poco::Any Saveable::GetAdapter(const std::string&  /*adapter*/)
{
  return Poco::Any();
}

}
