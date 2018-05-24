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

#ifndef QmitkUndoAction_H_
#define QmitkUndoAction_H_

#include <QAction>
#include <QIcon>

#include <berryIWorkbenchWindow.h>

#include <org_mitk_gui_qt_application_Export.h>

#include <mitkIDataStorageReference.h>

class QmitkUndoActionPrivate;

/**
* \ingroup org_mitk_gui_qt_application
*/
class MITK_QT_APP QmitkUndoAction : public QAction
{
  Q_OBJECT

public:
  QmitkUndoAction(berry::IWorkbenchWindow::Pointer window);
  QmitkUndoAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window);
  QmitkUndoAction(const QIcon & icon, berry::IWorkbenchWindow* window);

  ~QmitkUndoAction() override;

  protected slots:

    virtual void Run();

private:

  const QScopedPointer<QmitkUndoActionPrivate> d;
};

#endif /*QmitkUndoAction_H_*/
