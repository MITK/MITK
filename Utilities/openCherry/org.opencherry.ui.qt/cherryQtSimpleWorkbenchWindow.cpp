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
  m_WorkbenchPage = new WorkbenchPage(this, "", 0);
  
  IViewRegistry* viewRegistry = PlatformUI::GetWorkbench()->GetViewRegistry();
  const std::vector<IViewDescriptor::Pointer>& viewDescriptors = viewRegistry->GetViews();
  
  m_ViewMenu = menuBar()->addMenu("Show &View");
  
  std::vector<IViewDescriptor::Pointer>::const_iterator iter;
  for (iter = viewDescriptors.begin(); iter != viewDescriptors.end(); ++iter)
  {
    QtShowViewAction* viewAction = new QtShowViewAction(this, *iter);
    m_ViewActions.push_back(viewAction);
    m_ViewMenu->addAction(viewAction);
  }
  
  menuBar()->addSeparator();
  
  m_HelpMenu = menuBar()->addMenu("&Help");
  
  //m_HelpMenu->addAction(aboutAction);
  
  
  m_EditorTabs = new QTabWidget(this);
  this->setCentralWidget(m_EditorTabs);
  
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
