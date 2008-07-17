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

#include <cherryIWorkbenchPage.h>
#include <cherryIViewRegistry.h>
#include <cherryPlatformUI.h>

#include <QtGui/QMenuBar>
#include <QtGui/QDockWidget>
#include <QtGui/QBoxLayout>

namespace cherry {

QtWorkbenchWindow::QtWorkbenchWindow(int number)
 : WorkbenchWindow(number)
{
  // set the shell style
  //this->setWindowFlags(this->GetWindowConfigurer()->GetShellStyle());

  Point size = this->GetInitialSize();
  this->resize(size.x, size.y);
}

void* QtWorkbenchWindow::GetMenuManager()
{
  return menuBar();
}

int QtWorkbenchWindow::OpenImpl()
{
  this->show();
  return 0;
}

void QtWorkbenchWindow::CreateDefaultContents(void* shell)
{

}

void* QtWorkbenchWindow::GetPageComposite()
{
  return this;
}

QtWorkbenchWindow::~QtWorkbenchWindow()
{
  std::vector<QtShowViewAction*>::iterator iter;
  for (iter = m_ViewActions.begin(); iter != m_ViewActions.end(); ++iter)
  {
    delete (*iter);
  }
}

}
