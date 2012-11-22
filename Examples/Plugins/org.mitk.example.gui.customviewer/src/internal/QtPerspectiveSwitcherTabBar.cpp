/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QtPerspectiveSwitcherTabBar.h"

#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIPerspectiveDescriptor.h>

/**
 * \brief A Listener class for perspective changes. Neccessary for consistent tab activation
 * in QtPerspectiveSwitcherTabBar instances.
 */
// //! [SwitchPerspectiveListener]
struct QtPerspectiveSwitcherTabBarListener : public berry::IPerspectiveListener
// //! [SwitchPerspectiveListener]
{
  /**
   * Constructor.
   */
  QtPerspectiveSwitcherTabBarListener(QtPerspectiveSwitcherTabBar* switcher)
  : switcher(switcher)
  {}

  /**
   * Only listens to perspective activation events.
   */
  Events::Types GetPerspectiveEventTypes() const
  {
    return Events::ACTIVATED;
  }

  /**
   * Sets the corresponding perspective index within the associated QtPerspectiveSwitcherTabBar instance.
   */
// //! [SwitchPerspectiveListenerPerspectiveActivated]
  void PerspectiveActivated(berry::IWorkbenchPage::Pointer /*page*/,
    berry::IPerspectiveDescriptor::Pointer perspective)
  {
    int index = perspective->GetId() == "org.mitk.example.viewerperspective" ? 0 : 1;
    switcher->setCurrentIndex(index);
  }
// //! [SwitchPerspectiveListenerPerspectiveActivated]

private:

  /**
   *  The associated QtPerspectiveSwitcherTabBar instance.
   */
  QtPerspectiveSwitcherTabBar* switcher;
};

QtPerspectiveSwitcherTabBar::QtPerspectiveSwitcherTabBar(berry::IWorkbenchWindow::Pointer window)
: window(window)
{
  this->tabChanged = false;

  QWidget* parent = static_cast<QWidget*>(window->GetShell()->GetControl());
  this->setParent(parent);

  QObject::connect( this, SIGNAL( currentChanged( int ) )
    , this, SLOT( SwitchPerspective( void ) ) );

  this->perspListener = new QtPerspectiveSwitcherTabBarListener(this);
  window->AddPerspectiveListener(this->perspListener);
}

QtPerspectiveSwitcherTabBar::~QtPerspectiveSwitcherTabBar()
{
}

// //! [PerspectiveSwitcherSwitchPerspective]
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
    std::string perspectiveId = "org.mitk.example.viewerperspective";
    this->window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
  }
  else if (index == 1)
  {
    std::string perspectiveId = "org.mitk.example.dicomperspective";
    this->window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
  }
}
// //! [PerspectiveSwitcherSwitchPerspective]
