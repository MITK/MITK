/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKFILESAVEACTION_H_
#define QMITKFILESAVEACTION_H_

#include <org_mitk_gui_qt_application_Export.h>

#include <berryIWorkbenchWindow.h>

// qt
#include <QAction>
#include <QIcon>

class QmitkFileSaveActionPrivate;

class MITK_QT_APP QmitkFileSaveAction : public QAction
{
  Q_OBJECT

public:

  QmitkFileSaveAction(berry::IWorkbenchWindow::Pointer window);
  QmitkFileSaveAction(const QIcon& icon, berry::IWorkbenchWindow::Pointer window);
  QmitkFileSaveAction(const QIcon& icon, berry::IWorkbenchWindow* window);

  ~QmitkFileSaveAction() override;

protected slots:

  virtual void Run();

private:

  const QScopedPointer<QmitkFileSaveActionPrivate> d;

};


#endif /*QMITKFILESAVEACTION_H_*/
