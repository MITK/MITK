/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center, 
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without 
even the implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QtPerspectiveSwitcherTabBar.h"

//#include <berryQtOpenPerspectiveAction.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIPerspectiveRegistry.h>

#include <QActionGroup>

//namespace berry {

//struct QtPerspectiveSwitcherTabBarListener : public IPerspectiveListener
//{
//  QtPerspectiveSwitcherTabBarListener(QtPerspectiveSwitcherTabBar* switcher)
//  : switcher(switcher)
//  {}
//
//  Events::Types GetPerspectiveEventTypes() const
//  {
//    return Events::ACTIVATED;
//  }
//
//  void PerspectiveActivated(IWorkbenchPage::Pointer /*page*/,
//          IPerspectiveDescriptor::Pointer perspective)
//  {
//    QAction* action = switcher->perspIdToActionMap[QString::fromStdString(perspective->GetId())];
//    if (action) action->setChecked(true);
//  }
//
//private:
//
//  QtPerspectiveSwitcherTabBar* switcher;
//};


QtPerspectiveSwitcherTabBar::QtPerspectiveSwitcherTabBar(berry::IWorkbenchWindow::Pointer window)
: window(window)
{
  this->tabChanged = false;

  QWidget* parent = static_cast<QWidget*>(window->GetShell()->GetControl());
  this->setParent(parent);

  //this->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  QObject::connect( this, SIGNAL( currentChanged( int ) )
    , this, SLOT( SwitchPerspective( void ) ) );

  //IPerspectiveRegistry* perspRegistry = window->GetWorkbench()->GetPerspectiveRegistry();

  //std::vector<IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());
}

QtPerspectiveSwitcherTabBar::~QtPerspectiveSwitcherTabBar()
{
//  window->RemovePerspectiveListener(perspListener);
}

void QtPerspectiveSwitcherTabBar::SwitchPerspective()
{
  if (!this->tabChanged)
  {
    this->tabChanged = true;
    return;
  }

  int index = this->currentIndex();
  if (index == 0)
  {
    std::string perspectiveId = "org.mitk.example.testperspective";
    this->window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
  }
  else if (index == 1)
  {
    std::string perspectiveId = "org.mitk.example.testperspective2";
    this->window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
  }
}
