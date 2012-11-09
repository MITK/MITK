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
  std::string perspectiveId;

};

}

#endif /* BERRYQTOPENPERSPECTIVEACTION_H_ */
