/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
