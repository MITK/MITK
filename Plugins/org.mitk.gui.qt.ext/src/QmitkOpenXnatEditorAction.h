/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKOPENXNATEDITORACTION_H_
#define QMITKOPENXNATEDITORACTION_H_

#include <QAction>
#include <QIcon>

#include <org_mitk_gui_qt_ext_Export.h>

#include <berryIWorkbenchWindow.h>
#include <berryIPreferences.h>

class MITK_QT_COMMON_EXT_EXPORT QmitkOpenXnatEditorAction : public QAction
{
  Q_OBJECT

public:
  QmitkOpenXnatEditorAction(berry::IWorkbenchWindow::Pointer window);
  QmitkOpenXnatEditorAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window);

protected slots:

  void Run();

private:
  void init ( berry::IWorkbenchWindow::Pointer window );
  berry::IWorkbenchWindow::Pointer m_Window;
  berry::IPreferences::WeakPtr m_GeneralPreferencesNode;
};


#endif /*QMITKOPENXNATEDITORACTION_H_*/
