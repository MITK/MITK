/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QtPerspectiveSwitcherTabBar.h"

#include <berryIPerspectiveDescriptor.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>

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
  QtPerspectiveSwitcherTabBarListener(QtPerspectiveSwitcherTabBar *switcher) : switcher(switcher) {}
  /**
   * Only listens to perspective activation events.
   */
  Events::Types GetPerspectiveEventTypes() const override { return Events::ACTIVATED; }
  /**
   * Sets the corresponding perspective index within the associated QtPerspectiveSwitcherTabBar instance.
   */
  // //! [SwitchPerspectiveListenerPerspectiveActivated]
  void PerspectiveActivated(const berry::IWorkbenchPage::Pointer & /*page*/,
                            const berry::IPerspectiveDescriptor::Pointer &perspective) override
  {
    int index = perspective->GetId() == "org.mitk.example.viewerperspective" ? 0 : 1;
    switcher->setCurrentIndex(index);
  }
  // //! [SwitchPerspectiveListenerPerspectiveActivated]

private:
  /**
   *  The associated QtPerspectiveSwitcherTabBar instance.
   */
  QtPerspectiveSwitcherTabBar *switcher;
};

QtPerspectiveSwitcherTabBar::QtPerspectiveSwitcherTabBar(berry::IWorkbenchWindow::Pointer window)
  : window(window), perspListener(new QtPerspectiveSwitcherTabBarListener(this))
{
  this->tabChanged = false;

  QWidget *parent = static_cast<QWidget *>(window->GetShell()->GetControl());
  this->setParent(parent);

  QObject::connect(this, SIGNAL(currentChanged(int)), this, SLOT(SwitchPerspective(void)));

  window->AddPerspectiveListener(this->perspListener.data());
}

QtPerspectiveSwitcherTabBar::~QtPerspectiveSwitcherTabBar()
{
  window->RemovePerspectiveListener(this->perspListener.data());
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
    QString perspectiveId = "org.mitk.example.viewerperspective";
    this->window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
  }
  else if (index == 1)
  {
    QString perspectiveId = "org.mitk.example.dicomperspective";
    this->window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
  }
}
// //! [PerspectiveSwitcherSwitchPerspective]
