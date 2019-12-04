/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTOPENPERSPECTIVEACTION_H_
#define BERRYQTOPENPERSPECTIVEACTION_H_

#include <QAction>
#include <QActionGroup>

#include <berryIWorkbenchWindow.h>
#include <berryIPerspectiveDescriptor.h>

//TODO should be removed later
#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

class BERRY_UI_QT QtOpenPerspectiveAction: public QAction
{
  Q_OBJECT

public:

  QtOpenPerspectiveAction(IWorkbenchWindow::Pointer window,
      IPerspectiveDescriptor::Pointer descr, QActionGroup* group);

protected slots:

  void Run();

private:

  IWorkbenchWindow* window;
  QString perspectiveId;

};

}

#endif /* BERRYQTOPENPERSPECTIVEACTION_H_ */
