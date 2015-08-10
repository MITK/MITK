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

#ifndef QmitkCloseProjectAction_H_
#define QmitkCloseProjectAction_H_

#ifdef __MINGW32__
// We need to include winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <berrySmartPointer.h>

#include <QAction>

#include <org_mitk_gui_qt_application_Export.h>

namespace berry {
struct IWorkbenchWindow;
}

#include <mitkSceneIO.h>

/**
 * \ingroup org_mitk_gui_qt_application
 */
class MITK_QT_APP QmitkCloseProjectAction : public QAction
{
  Q_OBJECT

public:
  QmitkCloseProjectAction(berry::SmartPointer<berry::IWorkbenchWindow> window, mitk::SceneIO::Pointer sceneIO);
  QmitkCloseProjectAction(berry::IWorkbenchWindow* window, mitk::SceneIO::Pointer sceneIO);
  QmitkCloseProjectAction(const QIcon & icon, berry::SmartPointer<berry::IWorkbenchWindow> window, mitk::SceneIO::Pointer sceneIO);
  QmitkCloseProjectAction(const QIcon & icon, berry::IWorkbenchWindow* window, mitk::SceneIO::Pointer sceneIO);
public slots:
  bool Run();
signals:
  void projectClosed();

private:
  void init(berry::IWorkbenchWindow* window, mitk::SceneIO::Pointer sceneIO);
  berry::IWorkbenchWindow* m_Window;
  mitk::SceneIO::Pointer m_SceneIO;
};
#endif /*QmitkCloseProjectAction_H_*/
