/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
  ~QtPerspectiveSwitcher() override;

private:

  IWorkbenchWindow::Pointer window;
  QScopedPointer<IPerspectiveListener> perspListener;

  QHash<QString, QAction*> perspIdToActionMap;

  friend struct QtPerspectiveSwitcherListener;
};

}

#endif /* BERRYQTPERSPECTIVESWITCHER_H_ */
