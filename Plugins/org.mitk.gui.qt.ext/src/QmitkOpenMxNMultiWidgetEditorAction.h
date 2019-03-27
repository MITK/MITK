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

#ifndef QMITKOPENMXNMULTIWIDGETEDITORACTION_H
#define QMITKOPENMXNMULTIWIDGETEDITORACTION_H

#ifdef __MINGW32__
// We need to include winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwise they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QAction>
#include <QIcon>

#include <org_mitk_gui_qt_ext_Export.h>

#include <berryIWorkbenchWindow.h>
#include <berryIPreferences.h>

class MITK_QT_COMMON_EXT_EXPORT QmitkOpenMxNMultiWidgetEditorAction : public QAction
{
  Q_OBJECT

public:

  QmitkOpenMxNMultiWidgetEditorAction(berry::IWorkbenchWindow::Pointer window);
  QmitkOpenMxNMultiWidgetEditorAction(const QIcon& icon, berry::IWorkbenchWindow::Pointer window);

protected slots:

  void Run();

private:

  void init(berry::IWorkbenchWindow::Pointer window);
  berry::IWorkbenchWindow::Pointer m_Window;
  berry::IPreferences::WeakPtr m_GeneralPreferencesNode;

};

#endif // QMITKOPENMXNMULTIWIDGETEDITORACTION_H
