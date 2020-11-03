/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRedoAction_H_
#define QmitkRedoAction_H_

#include <QAction>
#include <QIcon>

#include <berryIWorkbenchWindow.h>

#include <org_mitk_gui_qt_application_Export.h>

#include <mitkIDataStorageReference.h>

class QmitkRedoActionPrivate;

/**
* \ingroup org_mitk_gui_qt_application
*/
class MITK_QT_APP QmitkRedoAction : public QAction
{
  Q_OBJECT

public:
  QmitkRedoAction(berry::IWorkbenchWindow::Pointer window);
  QmitkRedoAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window);
  QmitkRedoAction(const QIcon & icon, berry::IWorkbenchWindow* window);

  ~QmitkRedoAction() override;

  protected slots:

    virtual void Run();

private:

  const QScopedPointer<QmitkRedoActionPrivate> d;
};

#endif /*QmitkRedoAction_H_*/
