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

#ifndef QMITKFILEOPENACTION_H_
#define QMITKFILEOPENACTION_H_

#include <org_mitk_gui_qt_application_Export.h>

#include <berryIWorkbenchWindow.h>

// qt
#include <QAction>
#include <QIcon>

class QmitkFileOpenActionPrivate;

class MITK_QT_APP QmitkFileOpenAction : public QAction
{
  Q_OBJECT

public:

  QmitkFileOpenAction(berry::IWorkbenchWindow::Pointer window);
  QmitkFileOpenAction(const QIcon& icon, berry::IWorkbenchWindow::Pointer window);
  QmitkFileOpenAction(const QIcon& icon, berry::IWorkbenchWindow* window);

  virtual ~QmitkFileOpenAction() override;

protected slots:

  virtual void Run();

private:

  const QScopedPointer<QmitkFileOpenActionPrivate> d;

};


#endif /*QMITKFILEOPENACTION_H_*/
