/*=========================================================================
 
Program:   openCherry Platform
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

#include <cherryQtSimpleWorkbenchWindow.h>

class QmitkFileOpenAction : public QAction
{
  Q_OBJECT
  
public:
  
  QmitkFileOpenAction(cherry::QtWorkbenchWindow::Pointer window);
  
protected slots:
  
  void Run();
  
private:
  
  cherry::QtWorkbenchWindow::Pointer m_Window;
};


#endif /*QMITKFILEOPENACTION_H_*/
