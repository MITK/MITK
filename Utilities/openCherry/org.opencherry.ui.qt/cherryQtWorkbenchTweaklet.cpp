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

#include "cherryQtWorkbenchTweaklet.h"

#include <QApplication>

#include "internal/cherryQtShowViewDialog.h"
#include "internal/cherryQtViewPane.h"
#include "internal/cherryQtEditorPane.h"
#include "internal/cherryQtErrorView.h"

#include "cherryQtSimpleWorkbenchWindow.h"

#include <cherryPlatform.h>
#include <cherryPlatformUI.h>

namespace cherry {

QtWorkbenchTweaklet::QtWorkbenchTweaklet()
{

}

QtWorkbenchTweaklet::~QtWorkbenchTweaklet()
{
  //if (m_EditorPresentation != 0) delete m_EditorPresentation;
}

int
QtWorkbenchTweaklet::RunEventLoop()
{

  // spin Qt event loop
  return QApplication::instance()->exec();

}

bool QtWorkbenchTweaklet::IsRunning()
{
  return QApplication::instance() != 0;
}

WorkbenchWindow::Pointer QtWorkbenchTweaklet::CreateWorkbenchWindow(int newWindowNumber)
{
  WorkbenchWindow::Pointer window = new QtWorkbenchWindow(newWindowNumber);
  return window;
}

IDialog::Pointer
QtWorkbenchTweaklet::CreateStandardDialog(const std::string& dialogid)
{
  if (dialogid == DIALOG_ID_SHOW_VIEW)
    return new QtShowViewDialog(PlatformUI::GetWorkbench()->GetViewRegistry());
  else
    return IDialog::Pointer(0);
}

IViewPart::Pointer QtWorkbenchTweaklet::CreateErrorViewPart(const std::string& partName, const std::string& msg)
{
  QtErrorView::Pointer view = new QtErrorView();
  view->SetErrorMsg(msg);
  return view;
}

IEditorPart::Pointer QtWorkbenchTweaklet::CreateErrorEditorPart(const std::string& partName, const std::string& msg)
{
  return 0;
}

PartPane::Pointer QtWorkbenchTweaklet::CreateViewPane(IWorkbenchPartReference::Pointer partReference,
    WorkbenchPage::Pointer workbenchPage)
{
  PartPane::Pointer pane = new QtViewPane(partReference, workbenchPage);
  return pane;
}

PartPane::Pointer QtWorkbenchTweaklet::CreateEditorPane(IWorkbenchPartReference::Pointer partReference,
    WorkbenchPage::Pointer workbenchPage)
{
  PartPane::Pointer pane = new QtEditorPane(partReference, workbenchPage);
  return pane;
}

IEditorAreaHelper* QtWorkbenchTweaklet::CreateEditorPresentation(IWorkbenchPage::Pointer page)
{
  return new QtSimpleEditorAreaHelper(page);
}

void QtWorkbenchTweaklet::AddViewPane(IWorkbenchWindow::Pointer window, PartPane::Pointer pane)
{
  std::cout << "Adding view pane " << pane->GetPartReference()->GetPartName() << " to window\n";
  // temporary hack with dock widgets.
  QWidget* control = static_cast<QWidget*>(pane->GetControl());
  window.Cast<QtWorkbenchWindow>()->addDockWidget(Qt::RightDockWidgetArea, dynamic_cast<QDockWidget*>(control->parent()));
}

}  // namespace cherry
