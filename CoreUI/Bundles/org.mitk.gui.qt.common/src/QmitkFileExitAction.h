/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-08-04 15:43:53 +0200 (Di, 04 Aug 2009) $
Version:   $Revision: 18435 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKFILEEXITACTION_H_
#define QMITKFILEEXITACTION_H_

#include <QAction>

#include "mitkQtCommonDll.h"

#include <berryIWorkbenchWindow.h>

class MITK_QT_COMMON QmitkFileExitAction : public QAction
{
  Q_OBJECT

public:

  QmitkFileExitAction(berry::IWorkbenchWindow::Pointer window);

protected slots:

  void Run();

private:

  berry::IWorkbenchWindow::Pointer m_Window;
};


#endif /*QMITKFILEOPENACTION_H_*/
