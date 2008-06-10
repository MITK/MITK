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

#include "cherryQtSimpleWorkbenchWindow.h"

#include "org.opencherry.ui/src/cherryIViewRegistry.h"

#include "org.opencherry.ui/src/cherryPlatformUI.h"

#include <QtGui/QMenuBar>
#include <QtGui/QDockWidget>
#include <QtGui/QBoxLayout>

namespace cherry {

QtWorkbenchWindow::QtWorkbenchWindow()
{
  // set the shell style
  //this->setWindowFlags(this->GetWindowConfigurer()->GetShellStyle());
  
  m_WorkbenchPage = new WorkbenchPage(this, "", 0);
  
  Point size = this->GetInitialSize();
  this->resize(size.x, size.y);
}

void* QtWorkbenchWindow::GetMenuManager()
{
  return menuBar();
}

void QtWorkbenchWindow::CreateDefaultContents(void* shell)
{
  
}

QtWorkbenchWindow::~QtWorkbenchWindow()
{
  std::vector<QtShowViewAction*>::iterator iter;
  for (iter = m_ViewActions.begin(); iter != m_ViewActions.end(); ++iter)
  {
    delete (*iter);
  }
}

IWorkbenchPage::Pointer QtWorkbenchWindow::GetActivePage()
{
  return m_WorkbenchPage;
}

}
