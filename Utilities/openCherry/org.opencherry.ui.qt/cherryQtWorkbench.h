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

#ifndef CHERRYQTWORKBENCH_H_
#define CHERRYQTWORKBENCH_H_

#include <cherryWorkbench.h>

#include "internal/cherryQtSimpleEditorAreaHelper.h"

#include "cherryQtSimpleWorkbenchWindow.h"
#include "cherryUiQtDll.h"

namespace cherry {

class CHERRY_UI_QT QtWorkbench : public Workbench
{
  
public:
  
  cherryClassMacro(QtWorkbench);
  
  QtWorkbench();
  ~QtWorkbench();
  
  IWorkbenchWindow::Pointer GetActiveWorkbenchWindow();
  IDialog::Pointer CreateStandardDialog(const std::string& dialogid);
  IViewPart::Pointer CreateErrorViewPart(const std::string& partName = "", const std::string& msg = "");
  IEditorPart::Pointer CreateErrorEditorPart(const std::string& partName = "", const std::string& msg = "");
  
  PartPane::Pointer CreateViewPane(IWorkbenchPartReference::Pointer partReference,
      WorkbenchPage::Pointer workbenchPage);
  PartPane::Pointer CreateEditorPane(IWorkbenchPartReference::Pointer partReference,
      WorkbenchPage::Pointer workbenchPage);
  
  IEditorAreaHelper* CreateEditorPresentation(IWorkbenchPage::Pointer page);
  
  void* CreateWorkbenchPageControl();
  
  void AddViewPane(PartPane::Pointer pane);
  
  
protected:
  
  int RunUI();
  
  void OpenFirstTimeWindow();
  
  /**
   * Initializes the workbench 
   * 
   * @return true if init succeeded.
   */
  bool Init();
  
  IWorkbenchWindow::Pointer RestoreWorkbenchWindow(IMemento::Pointer memento);
  
  
private:
  
  QtWorkbenchWindow::Pointer m_QtWindow;
  
  QtSimpleEditorAreaHelper* m_EditorPresentation;
  
};

} // namespace cherry

#endif /*CHERRYQTWORKBENCH_H_*/
