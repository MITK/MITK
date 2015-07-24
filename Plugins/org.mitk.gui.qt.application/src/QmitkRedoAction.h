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

#ifndef QmitkRedoAction_H_
#define QmitkRedoAction_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

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

  ~QmitkRedoAction();

  protected slots:

    virtual void Run();

private:

  const QScopedPointer<QmitkRedoActionPrivate> d;
};

#endif /*QmitkRedoAction_H_*/
