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

#ifndef QmitkExtFileOpenProjectAction_H_
#define QmitkExtFileOpenProjectAction_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QAction>

#include <org_mitk_gui_qt_ext_Export.h>

#include <berrySmartPointer.h>
#include <mitkSceneIO.h>

namespace berry {
struct IWorkbenchWindow;
}

class MITK_QT_COMMON_EXT_EXPORT QmitkExtFileOpenProjectAction : public QAction
{
  Q_OBJECT

public:

  QmitkExtFileOpenProjectAction(berry::SmartPointer<berry::IWorkbenchWindow> window, mitk::SceneIO::Pointer sceneIO);
  QmitkExtFileOpenProjectAction(berry::IWorkbenchWindow* window, mitk::SceneIO::Pointer sceneIO);

  mitk::SceneIO::Pointer GetSceneIO() { return m_SceneIO; }

public slots:

  void Run(QString fileName = QString(""));

signals:
  void projectOpened(QString fileName);
  void projectOpenFailed(QString fileName);

private:
  void init(berry::IWorkbenchWindow* window, mitk::SceneIO::Pointer sceneIO);

  berry::IWorkbenchWindow* m_Window;
  mitk::SceneIO::Pointer m_SceneIO;
};


#endif /*QmitkExtFileOpenProjectAction_H_*/
