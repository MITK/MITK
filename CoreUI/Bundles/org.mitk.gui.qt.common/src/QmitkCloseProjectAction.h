/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-09 16:36:23 +0200 (Fr, 09 Okt 2009) $
Version:   $Revision: 19414 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkCloseProjectAction_H_
#define QmitkCloseProjectAction_H_

#ifdef __MINGW32__
// We need to include winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QAction>

#include "mitkQtCommonDll.h"

#include <berryIWorkbenchWindow.h>

class MITK_QT_COMMON QmitkCloseProjectAction : public QAction
{
  Q_OBJECT

public:
  QmitkCloseProjectAction(berry::IWorkbenchWindow::Pointer window);
  QmitkCloseProjectAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window);
protected slots:
  void Run();

private:
  void init(berry::IWorkbenchWindow::Pointer window);
  berry::IWorkbenchWindow::Pointer m_Window;
};
#endif /*QmitkCloseProjectAction_H_*/
