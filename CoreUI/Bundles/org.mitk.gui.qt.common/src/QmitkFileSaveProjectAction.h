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

#ifndef QmitkFileSaveProjectAction_H_
#define QmitkFileSaveProjectAction_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QAction>

#include "mitkQtCommonDll.h"

#include <cherryIWorkbenchWindow.h>

class MITK_QT_COMMON QmitkFileSaveProjectAction : public QAction
{
  Q_OBJECT

public:

  QmitkFileSaveProjectAction(cherry::IWorkbenchWindow::Pointer window);

protected slots:

  void Run();

private:

  cherry::IWorkbenchWindow::Pointer m_Window;
};


#endif /*QmitkFileSaveProjectAction_H_*/
