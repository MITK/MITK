/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOpenStdMultiWidgetEditorAction_h
#define QmitkOpenStdMultiWidgetEditorAction_h

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QAction>
#include <QIcon>

#include <org_mitk_gui_qt_ext_Export.h>

#include <berryIWorkbenchWindow.h>

class MITK_QT_COMMON_EXT_EXPORT QmitkOpenStdMultiWidgetEditorAction : public QAction
{
  Q_OBJECT

public:

  QmitkOpenStdMultiWidgetEditorAction(berry::IWorkbenchWindow::Pointer window);
  QmitkOpenStdMultiWidgetEditorAction(const QIcon& icon, berry::IWorkbenchWindow::Pointer window);

protected slots:

  void Run();

private:

  void init(berry::IWorkbenchWindow::Pointer window);
  berry::IWorkbenchWindow::Pointer m_Window;

};

#endif
