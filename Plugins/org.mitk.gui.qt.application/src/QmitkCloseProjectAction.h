/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCloseProjectAction_H_
#define QmitkCloseProjectAction_H_

#include <berrySmartPointer.h>

#include <QAction>

#include <org_mitk_gui_qt_application_Export.h>

namespace berry {
struct IWorkbenchWindow;
}

/**
 * \ingroup org_mitk_gui_qt_application
 */
class MITK_QT_APP QmitkCloseProjectAction : public QAction
{
  Q_OBJECT

public:
  QmitkCloseProjectAction(berry::SmartPointer<berry::IWorkbenchWindow> window);
  QmitkCloseProjectAction(berry::IWorkbenchWindow* window);
  QmitkCloseProjectAction(const QIcon & icon, berry::SmartPointer<berry::IWorkbenchWindow> window);
  QmitkCloseProjectAction(const QIcon & icon, berry::IWorkbenchWindow* window);
protected slots:
  void Run();

private:
  void init(berry::IWorkbenchWindow* window);
  berry::IWorkbenchWindow* m_Window;
};
#endif /*QmitkCloseProjectAction_H_*/
