/*=========================================================================

Program:   openCherry Platform
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

#ifndef CHERRYQTWORKBENCHTWEAKLET_H_
#define CHERRYQTWORKBENCHTWEAKLET_H_

#include <tweaklets/cherryWorkbenchTweaklet.h>

#include "internal/cherryQtSimpleEditorAreaHelper.h"

#include "cherryQtSimpleWorkbenchWindow.h"
#include "cherryUiQtDll.h"

namespace cherry {

class CHERRY_UI_QT QtWorkbenchTweaklet : public WorkbenchTweaklet
{

public:

  cherryClassMacro(QtWorkbenchTweaklet);

  QtWorkbenchTweaklet();
  ~QtWorkbenchTweaklet();

  int RunEventLoop();
  bool IsRunning();

  WorkbenchWindow::Pointer CreateWorkbenchWindow(int newWindowNumber);

  IEditorAreaHelper* CreateEditorPresentation(IWorkbenchPage::Pointer page);

  IDialog::Pointer CreateStandardDialog(const std::string& dialogid);

  IViewPart::Pointer CreateErrorViewPart(const std::string& partName = "", const std::string& msg = "");
  IEditorPart::Pointer CreateErrorEditorPart(const std::string& partName = "", const std::string& msg = "");

  PartPane::Pointer CreateViewPane(IWorkbenchPartReference::Pointer partReference,
      WorkbenchPage::Pointer workbenchPage);
  PartPane::Pointer CreateEditorPane(IWorkbenchPartReference::Pointer partReference,
      WorkbenchPage::Pointer workbenchPage);

  void AddViewPane(IWorkbenchWindow::Pointer window, PartPane::Pointer pane);

};

} // namespace cherry

#endif /*CHERRYQTWORKBENCHTWEAKLET_H_*/
