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

#include "cherryQtWorkbenchWindow.h"

#include "org.opencherry.ui/cherryIViewRegistry.h"

#include "org.opencherry.ui/cherryPlatformUI.h"

#include <QtGui/QMenuBar>
#include <QtGui/QDockWidget>
#include <QtGui/QBoxLayout>

namespace cherry {

QtWorkbenchWindow::QtWorkbenchWindow()
{
  IViewRegistry* viewRegistry = PlatformUI::GetWorkbench()->GetViewRegistry();
  const std::vector<IViewDescriptor::Pointer>& viewDescriptors = viewRegistry->GetViews();
  
  m_ViewMenu = menuBar()->addMenu("Show &View");
  
  std::vector<IViewDescriptor::Pointer>::const_iterator iter;
  for (iter = viewDescriptors.begin(); iter != viewDescriptors.end(); ++iter)
  {
    QtShowViewAction* viewAction = new QtShowViewAction(this, *iter);
    m_ViewActions.push_back(viewAction);
    m_ViewMenu->addAction(viewAction->GetQtAction());
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


QtShowViewAction::QtShowViewAction(QtWorkbenchWindow* workbench, IViewDescriptor::Pointer descriptor)
{
  m_Workbench = workbench;
  m_Descriptor = descriptor;
  m_Dock = 0;
  
  m_Action = new QAction(descriptor->GetLabel().c_str(), workbench);
  workbench->connect(m_Action, SIGNAL(triggered()), this, SLOT(ShowView()));
}

QtShowViewAction::~QtShowViewAction()
{
  delete m_Action;
  if (m_Dock != 0) delete m_Dock;
}
    
QAction*
QtShowViewAction::GetQtAction()
{
  return m_Action;
}
    
void
QtShowViewAction::ShowView()
{
  if (m_Dock == 0)
  {
    m_Dock = new QDockWidget(m_Descriptor->GetLabel().c_str(), m_Workbench);
    QWidget* controls = static_cast<QWidget*>(m_Descriptor->CreateView()->CreatePartControl(m_Dock));
    m_Dock->setWidget(controls);
    m_Workbench->addDockWidget(Qt::RightDockWidgetArea, m_Dock);
  }
  m_Dock->show();
}

}
