/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QTPERSPECTIVESWITCHERTABBAR_H_
#define QTPERSPECTIVESWITCHERTABBAR_H_

#include <QHash>
#include <QTabBar>

#include <berryIPerspectiveListener.h>
#include <berryIWorkbenchWindow.h>

/**
 * \brief A QTabBar providing perspective bar functionality in BlueBerry applications.
 *
 * This subclass of QTabBar acts as a perspective bar in BlueBerry applications. Providing
 * perspective switching functionality in a tab-bar like outfit, this class serves as an
 * alternative to the ToolBar based berry::QtPerspectiveSwitcher class.
 */
// //! [PerspectiveSwitcherDeclaration]
class QtPerspectiveSwitcherTabBar : public QTabBar
// //! [PerspectiveSwitcherDeclaration]
{
  Q_OBJECT

public:
  /**
   * Constructor.
   */
  QtPerspectiveSwitcherTabBar(berry::IWorkbenchWindow::Pointer window);

  /**
   * Standard destructor.
   */
  ~QtPerspectiveSwitcherTabBar() override;

private Q_SLOTS:

  /**
   * Implements perspective switching.
   */
  void SwitchPerspective();

private:
  berry::IWorkbenchWindow::Pointer window;
  QScopedPointer<berry::IPerspectiveListener> perspListener;

  QHash<QString, QAction *> perspIdToActionMap;

  /**
   * Neccessary to prevent initial tab switching.
   */
  bool tabChanged;

  /**
   * Listener for perspective changes. Neccessary for consistent tab activation.
   */
  friend struct QtPerspectiveSwitcherTabBarListener;
};

#endif /* QTPERSPECTIVESWITCHERTABBAR_H_ */
