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

#include "cherryQtWorkbench.h"

#include <QApplication>

#include "internal/cherryQtShowViewDialog.h"
#include "internal/cherryQtViewPane.h"
#include "internal/cherryQtEditorPane.h"
#include "internal/cherryQtErrorView.h"

#include <cherryPlatform.h>
#include <cherryPlatformUI.h>

namespace cherry {

QtWorkbench::QtWorkbench()
 : m_EditorPresentation(0)
{
  
}

QtWorkbench::~QtWorkbench()
{
  if (m_EditorPresentation != 0) delete m_EditorPresentation;
}

IWorkbenchWindow::Pointer QtWorkbench::GetActiveWorkbenchWindow()
{
  return m_QtWindow;
}

bool QtWorkbench::Init() 
{    
  return Workbench::Init();
}

int
QtWorkbench::RunUI()
{
  const std::vector<std::string> args = Platform::GetApplicationArgs();
  int argc = args.size();
  char** argv = new char*[argc];
  
  int counter = 0;
  for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it, ++counter)
  {
    argv[counter] = const_cast<char*>(it->c_str());
  }
  
  QApplication qtApp(argc, argv);
  
  // initialize workbench and restore or open one window
  bool initOK = this->Init();
  
  // let the advisor run its start up code
  if (initOK) {
    advisor->PostStartup(); // may trigger a close/restart
  }


  //TODO start eager plug-ins
  //startPlugins();
    
  //addStartupRegistryListener();
  
  // spin Qt event loop
  qtApp.exec();
  
  delete[] argv;
  
  return PlatformUI::RETURN_OK;
}

void QtWorkbench::OpenFirstTimeWindow()
{
  m_QtWindow = new QtWorkbenchWindow();
  m_QtWindow->Init();
  m_QtWindow->show();
}

IWorkbenchWindow::Pointer QtWorkbench::RestoreWorkbenchWindow(IMemento::Pointer memento)
{
  return 0;
}

IDialog::Pointer
QtWorkbench::CreateStandardDialog(const std::string& dialogid)
{
  if (dialogid == DIALOG_ID_SHOW_VIEW)
    return new QtShowViewDialog(PlatformUI::GetWorkbench()->GetViewRegistry());
  else
    return IDialog::Pointer(0);
}

IViewPart::Pointer QtWorkbench::CreateErrorViewPart(const std::string& partName, const std::string& msg)
{
  QtErrorView::Pointer view = new QtErrorView();
  view->SetErrorMsg(msg);
  return view;
}

IEditorPart::Pointer QtWorkbench::CreateErrorEditorPart(const std::string& partName, const std::string& msg)
{
  return 0;
}

PartPane::Pointer QtWorkbench::CreateViewPane(IWorkbenchPartReference::Pointer partReference,
    WorkbenchPage::Pointer workbenchPage)
{
  PartPane::Pointer pane = new QtViewPane(partReference, workbenchPage);
  return pane;
}

PartPane::Pointer QtWorkbench::CreateEditorPane(IWorkbenchPartReference::Pointer partReference,
    WorkbenchPage::Pointer workbenchPage)
{
  PartPane::Pointer pane = new QtEditorPane(partReference, workbenchPage);
  return pane;
}

IEditorAreaHelper* QtWorkbench::CreateEditorPresentation(IWorkbenchPage::Pointer page)
{
  if (m_EditorPresentation == 0)
    m_EditorPresentation = new QtSimpleEditorAreaHelper(page);
  
  return m_EditorPresentation;
}

void* QtWorkbench::CreateWorkbenchPageControl()
{
  return m_QtWindow;
}

void QtWorkbench::AddViewPane(PartPane::Pointer pane)
{
  // temporary hack with dock widgets.
  QWidget* control = static_cast<QWidget*>(pane->GetControl());
  m_QtWindow->addDockWidget(Qt::RightDockWidgetArea, dynamic_cast<QDockWidget*>(control->parent()));
}

}  // namespace cherry
