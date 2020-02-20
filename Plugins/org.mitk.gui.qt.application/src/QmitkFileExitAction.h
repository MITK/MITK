/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKFILEEXITACTION_H_
#define QMITKFILEEXITACTION_H_

#include <berrySmartPointer.h>

#include <QAction>

#include <org_mitk_gui_qt_application_Export.h>

namespace berry {
  struct IWorkbenchWindow;
}

/**
 * \ingroup org_mitk_gui_qt_application
 */
class MITK_QT_APP QmitkFileExitAction : public QAction
{
  Q_OBJECT

public:

  QmitkFileExitAction(berry::SmartPointer<berry::IWorkbenchWindow> window);
  QmitkFileExitAction(berry::IWorkbenchWindow* window);
  QmitkFileExitAction(const QIcon & icon, berry::SmartPointer<berry::IWorkbenchWindow> window);
  QmitkFileExitAction(const QIcon & icon, berry::IWorkbenchWindow* window);

protected slots:

  void Run();

private:
  void init(berry::IWorkbenchWindow* window);
  berry::IWorkbenchWindow* m_Window;
};


#endif /*QMITKFILEOPENACTION_H_*/
