/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWorkbenchWindowConfigurer.h"

#include "berryWorkbenchWindow.h"
#include "berryWorkbench.h"
#include "berryWorkbenchPage.h"
#include "berryEditorSashContainer.h"
#include "berryWorkbenchPlugin.h"
#include "berryMenuManager.h"

#include "berryQtDnDControlWidget.h"

namespace berry
{

WorkbenchWindowConfigurer::WindowActionBarConfigurer::WindowActionBarConfigurer(WorkbenchWindow::WeakPtr wnd)
: window(wnd)
{

}

void WorkbenchWindowConfigurer::WindowActionBarConfigurer::SetProxy(IActionBarConfigurer::Pointer proxy)
{
  this->proxy = proxy;
}

IWorkbenchWindowConfigurer::Pointer WorkbenchWindowConfigurer::WindowActionBarConfigurer::GetWindowConfigurer()
{
  return WorkbenchWindow::Pointer(window)->GetWindowConfigurer();
}

IMenuManager* WorkbenchWindowConfigurer::WindowActionBarConfigurer::GetMenuManager()
{
  if (proxy.IsNotNull())
  {
    return proxy->GetMenuManager();
  }
  return window.Lock()->GetMenuManager();
}

IToolBarManager* WorkbenchWindowConfigurer::WindowActionBarConfigurer::GetToolBarManager()
{
  if (proxy.IsNotNull())
  {
    return proxy->GetToolBarManager();
  }
  //return window.Lock()->GetToolBarManager();
  return nullptr;
}

WorkbenchWindowConfigurer::WorkbenchWindowConfigurer(const WorkbenchWindow::Pointer& window)
 : shellStyle(nullptr)
 , showPerspectiveBar(false)
 , showStatusLine(true)
 , showToolBar(true)
 , showMenuBar(true)
 , showProgressIndicator(false)
 , dropTargetListener(nullptr)
 , initialSize(1024,768)
{
  if (window.IsNull())
  {
    throw Poco::InvalidArgumentException();
  }
  this->window = window;
  windowTitle = "BlueBerry Application";
}

IWorkbenchWindow::Pointer WorkbenchWindowConfigurer::GetWindow()
{
  return IWorkbenchWindow::Pointer(window);
}

IWorkbenchConfigurer::Pointer WorkbenchWindowConfigurer::GetWorkbenchConfigurer()
{
  return dynamic_cast<Workbench*>(PlatformUI::GetWorkbench())->GetWorkbenchConfigurer();
}

QString WorkbenchWindowConfigurer::BasicGetTitle()
{
  return windowTitle;
}

QString WorkbenchWindowConfigurer::GetTitle()
{
  Shell::Pointer shell = window.Lock()->GetShell();
  if (shell)
  {
    // update the cached title
    windowTitle = shell->GetText();
  }
  return windowTitle;
}

void WorkbenchWindowConfigurer::SetTitle(const QString &title)
{
  windowTitle = title;
  Shell::Pointer shell = window.Lock()->GetShell();
  if (shell)
  {
    shell->SetText(title);
  }
}

bool WorkbenchWindowConfigurer::GetShowMenuBar() const
{
  return showMenuBar;
}

void WorkbenchWindowConfigurer::SetShowMenuBar(bool show)
{
  showMenuBar = show;
//  WorkbenchWindow win = (WorkbenchWindow) getWindow();
//  Shell shell = win.getShell();
//  if (shell != null)
//  {
//    boolean showing = shell.getMenuBar() != null;
//    if (show != showing)
//    {
//      if (show)
//      {
//        shell.setMenuBar(win.getMenuBarManager().getMenu());
//      }
//      else
//      {
//        shell.setMenuBar(null);
//      }
//    }
//  }
}

bool WorkbenchWindowConfigurer::GetShowToolBar() const
{
  return showToolBar;
}

void WorkbenchWindowConfigurer::SetShowToolBar(bool show)
{
  showToolBar = show;
  //window.setCoolBarVisible(show);
  // @issue need to be able to reconfigure after window's controls created
}

bool WorkbenchWindowConfigurer::GetShowPerspectiveBar() const
{
  return showPerspectiveBar;
}

void WorkbenchWindowConfigurer::SetShowPerspectiveBar(bool show)
{
  showPerspectiveBar = show;
  //window.setPerspectiveBarVisible(show);
  // @issue need to be able to reconfigure after window's controls created
}

bool WorkbenchWindowConfigurer::GetShowStatusLine() const
{
  return showStatusLine;
}

void WorkbenchWindowConfigurer::SetShowStatusLine(bool show)
{
  showStatusLine = show;
  // @issue need to be able to reconfigure after window's controls created
}

bool WorkbenchWindowConfigurer::GetShowProgressIndicator() const
{
  return showProgressIndicator;
}

void WorkbenchWindowConfigurer::SetShowProgressIndicator(bool show)
{
  showProgressIndicator = show;
  // @issue need to be able to reconfigure after window's controls created
}

void WorkbenchWindowConfigurer::AddEditorAreaTransfer(const QStringList& transfers)
{
  if (transfers.isEmpty()) return;

  int oldSize = transferTypes.size();
  transferTypes.unite(QSet<QString>::fromList(transfers));

  if (transferTypes.size() == oldSize) return;

  WorkbenchPage::Pointer page = window.Lock()->GetActivePage().Cast<WorkbenchPage>();
  if (page)
  {
    QtDnDControlWidget* dropTarget =
        static_cast<QtDnDControlWidget*>(page->GetEditorPresentation()->GetLayoutPart().Cast<EditorSashContainer>()->GetParent());
    dropTarget->SetTransferTypes(transferTypes.toList());
  }
}

void WorkbenchWindowConfigurer::ConfigureEditorAreaDropListener(IDropTargetListener* listener)
{
  if (listener == nullptr) return;
  dropTargetListener = listener;

  WorkbenchPage::Pointer page = window.Lock()->GetActivePage().Cast<WorkbenchPage>();
  if (page)
  {
    QtDnDControlWidget* dropTarget =
        static_cast<QtDnDControlWidget*>(page->GetEditorPresentation()->GetLayoutPart().Cast<EditorSashContainer>()->GetParent());
    dropTarget->AddDropListener(listener);
  }
}

QStringList WorkbenchWindowConfigurer::GetTransfers() const
{
  return transferTypes.toList();
}

IDropTargetListener* WorkbenchWindowConfigurer::GetDropTargetListener() const
{
  return dropTargetListener;
}

IActionBarConfigurer::Pointer WorkbenchWindowConfigurer::GetActionBarConfigurer()
{
  if (actionBarConfigurer.IsNull())
  {
    // lazily initialize
    actionBarConfigurer = new WindowActionBarConfigurer(window);
  }
  return actionBarConfigurer;
}

Qt::WindowFlags WorkbenchWindowConfigurer::GetWindowFlags() const
{
  return shellStyle;
}

void WorkbenchWindowConfigurer::SetWindowFlags(Qt::WindowFlags shellStyle)
{
  this->shellStyle = shellStyle;
}

QPoint WorkbenchWindowConfigurer::GetInitialSize() const
{
  return initialSize;
}

void WorkbenchWindowConfigurer::SetInitialSize(QPoint size)
{
  initialSize = size;
}

void WorkbenchWindowConfigurer::CreateDefaultContents(Shell::Pointer shell)
{
  WorkbenchWindow::Pointer(window)->CreateDefaultContents(shell);
}

QMenuBar* WorkbenchWindowConfigurer::CreateMenuBar()
{
  return window.Lock()->GetMenuManager()->CreateMenuBar(window.Lock()->GetShell()->GetControl());
}

QWidget* WorkbenchWindowConfigurer::CreateToolBar(QWidget* /*parent*/)
{
//  IToolBarManager* toolBarManager = window->GetToolBarManager();
//  if (toolBarManager)
//  {
//    return toolBarManager->CreateControl(parent);
//  }
  return nullptr;
}

QWidget *WorkbenchWindowConfigurer::CreatePageComposite(QWidget *parent)
{
  return WorkbenchWindow::Pointer(window)->CreatePageComposite(parent);
}

bool WorkbenchWindowConfigurer::SaveState(IMemento::Pointer memento)
{
  return WorkbenchWindow::Pointer(window)->SaveState(memento);
}

}
