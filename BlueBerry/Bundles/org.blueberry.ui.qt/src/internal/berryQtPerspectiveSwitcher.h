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


#ifndef BERRYQTPERSPECTIVESWITCHER_H_
#define BERRYQTPERSPECTIVESWITCHER_H_

#include <QToolBar>
#include <QHash>

#include <berryIPerspectiveListener.h>
#include <berryIWorkbenchWindow.h>

namespace berry {

class QtPerspectiveSwitcher : public QToolBar
{
  Q_OBJECT

public:

  QtPerspectiveSwitcher(IWorkbenchWindow::Pointer window);
  ~QtPerspectiveSwitcher();

private:

  IWorkbenchWindow::Pointer window;
  IPerspectiveListener::Pointer perspListener;

  QHash<QString, QAction*> perspIdToActionMap;

  friend struct QtPerspectiveSwitcherListener;
};

}

#endif /* BERRYQTPERSPECTIVESWITCHER_H_ */
