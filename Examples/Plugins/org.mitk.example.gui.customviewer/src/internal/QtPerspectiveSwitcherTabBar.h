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


#ifndef QTPERSPECTIVESWITCHERTABBAR_H_
#define QTPERSPECTIVESWITCHERTABBAR_H_

#include <QTabBar>
#include <QHash>

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
  ~QtPerspectiveSwitcherTabBar();

private Q_SLOTS:

  /**
   * Implements perspective switching.
   */
  void SwitchPerspective();

private:

  berry::IWorkbenchWindow::Pointer window;
  berry::IPerspectiveListener::Pointer perspListener;

  QHash<QString, QAction*> perspIdToActionMap;

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
