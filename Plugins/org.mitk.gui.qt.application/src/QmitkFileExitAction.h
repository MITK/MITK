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

#ifndef QMITKFILEEXITACTION_H_
#define QMITKFILEEXITACTION_H_

#include <QAction>

#include <org_mitk_gui_qt_application_Export.h>

#include <berryIWorkbenchWindow.h>

/**
 * \ingroup org_mitk_gui_qt_application
 */
class MITK_QT_APP QmitkFileExitAction : public QAction
{
  Q_OBJECT

public:

  QmitkFileExitAction(berry::IWorkbenchWindow::Pointer window);

protected slots:

  void Run();

private:

  berry::IWorkbenchWindow* m_Window;
};


#endif /*QMITKFILEOPENACTION_H_*/
