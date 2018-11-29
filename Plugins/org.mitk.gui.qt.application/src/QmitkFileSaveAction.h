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

  virtual ~QmitkFileSaveAction() override;

protected slots:

  virtual void Run();

private:

  const QScopedPointer<QmitkFileSaveActionPrivate> d;

};


#endif /*QMITKFILESAVEACTION_H_*/
