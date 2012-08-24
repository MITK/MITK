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


#ifndef QTPERSPECTIVESWITCHERTABBAR_H_
#define QTPERSPECTIVESWITCHERTABBAR_H_

#include <QTabBar>
#include <QHash>

#include <berryIPerspectiveListener.h>
#include <berryIWorkbenchWindow.h>

class QtPerspectiveSwitcherTabBar : public QTabBar
{
  Q_OBJECT

public:

  QtPerspectiveSwitcherTabBar(berry::IWorkbenchWindow::Pointer window);
  ~QtPerspectiveSwitcherTabBar();

private slots:

  void SwitchPerspective();


private:

  berry::IWorkbenchWindow::Pointer window;
  berry::IPerspectiveListener::Pointer perspListener;

  QHash<QString, QAction*> perspIdToActionMap;

  bool tabChanged;

  friend struct QtPerspectiveSwitcherTabBarListener;
};


#endif /* QTPERSPECTIVESWITCHERTABBAR_H_ */
