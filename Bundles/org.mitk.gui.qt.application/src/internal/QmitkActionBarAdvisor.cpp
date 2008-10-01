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

#include "QmitkActionBarAdvisor.h"

#include <QMenu>
#include <QMenuBar>

#include <cherryPlatformUI.h>
#include <cherryIWorkbenchWindow.h>

#include <internal/cherryQtShowViewAction.h>

#include "QmitkFileOpenAction.h"

QmitkActionBarAdvisor::QmitkActionBarAdvisor(cherry::IActionBarConfigurer::Pointer configurer)
 : cherry::QtActionBarAdvisor(configurer)
{

}

void QmitkActionBarAdvisor::FillQtMenuBar(QMenuBar* menuBar)
{
//  cherry::IWorkbenchWindow::Pointer window = this->GetActionBarConfigurer()->GetWindowConfigurer()->GetWindow();
//
//  QMenu* fileMenu = menuBar->addMenu("&File");
//
//  fileMenu->addAction(new QmitkFileOpenAction(window));
//
//  cherry::IViewRegistry* viewRegistry = cherry::PlatformUI::GetWorkbench()->GetViewRegistry();
//  const std::vector<cherry::IViewDescriptor::Pointer>& viewDescriptors = viewRegistry->GetViews();
//
//  QMenu* viewMenu = menuBar->addMenu("Show &View");
//
//  std::vector<cherry::IViewDescriptor::Pointer>::const_iterator iter;
//  for (iter = viewDescriptors.begin(); iter != viewDescriptors.end(); ++iter)
//  {
//    cherry::QtShowViewAction* viewAction = new cherry::QtShowViewAction(window, *iter);
//    //m_ViewActions.push_back(viewAction);
//    viewMenu->addAction(viewAction);
//  }
}
