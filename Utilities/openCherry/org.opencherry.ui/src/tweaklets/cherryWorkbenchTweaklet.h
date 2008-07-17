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


#ifndef CHERRYWORKBENCHTWEAKLET_H_
#define CHERRYWORKBENCHTWEAKLET_H_

#include "../internal/cherryTweaklets.h"
#include "../cherryWorkbenchWindow.h"
#include "../cherryIEditorAreaHelper.h"
#include "../cherryIWorkbenchPage.h"
#include "../cherryPartPane.h"

namespace cherry
{

struct WorkbenchTweaklet : public Object
{
  cherryInterfaceMacro(WorkbenchTweaklet, cherry);

  static Tweaklets::TweakKey<WorkbenchTweaklet> KEY;

  static const std::string DIALOG_ID_SHOW_VIEW; // = "org.opencherry.ui.dialogs.showview";

  virtual int RunEventLoop() = 0;

  virtual bool IsRunning() = 0;

  virtual WorkbenchWindow::Pointer CreateWorkbenchWindow(int newWindowNumber) = 0;

  virtual IEditorAreaHelper* CreateEditorPresentation(IWorkbenchPage::Pointer page) = 0;

  virtual IDialog::Pointer CreateStandardDialog(const std::string& id) = 0;

  virtual IViewPart::Pointer CreateErrorViewPart(const std::string& partName = "", const std::string& msg = "") = 0;
  virtual IEditorPart::Pointer CreateErrorEditorPart(const std::string& partName = "", const std::string& msg = "") = 0;

  virtual PartPane::Pointer CreateViewPane(IWorkbenchPartReference::Pointer partReference,
      WorkbenchPage::Pointer workbenchPage) = 0;
  virtual PartPane::Pointer CreateEditorPane(IWorkbenchPartReference::Pointer partReference,
      WorkbenchPage::Pointer workbenchPage) = 0;

  // Hack for DummyWorkbenchPage
  virtual void AddViewPane(IWorkbenchWindow::Pointer window, PartPane::Pointer pane) = 0;
};

}

#endif /* CHERRYWORKBENCHTWEAKLET_H_ */
