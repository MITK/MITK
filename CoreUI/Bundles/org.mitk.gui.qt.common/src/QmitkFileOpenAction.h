/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKFILEOPENACTION_H_
#define QMITKFILEOPENACTION_H_

#include <QAction>

#include "mitkQtCommonDll.h"

#include <cherryIWorkbenchWindow.h>

class MITK_QT_COMMON QmitkFileOpenAction : public QAction
{
  Q_OBJECT

public:

  QmitkFileOpenAction(cherry::IWorkbenchWindow::Pointer window);

protected slots:

  void Run();

private:

  cherry::IWorkbenchWindow::Pointer m_Window;
};


#endif /*QMITKFILEOPENACTION_H_*/
