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


#include "QmitkWorkbenchAdvisor.h"

#include <QApplication>

#include <QmitkRegisterClasses.h>

#include "QmitkWorkbenchWindowAdvisor.h"

const std::string QmitkWorkbenchAdvisor::DEFAULT_PERSP_ID = "org.mitk.perspectives.default";

void
QmitkWorkbenchAdvisor::Initialize(cherry::IWorkbenchConfigurer::Pointer configurer)
{
  cherry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);
  QmitkRegisterClasses();
}

cherry::WorkbenchWindowAdvisor*
QmitkWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        cherry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  return new QmitkWorkbenchWindowAdvisor(configurer);
}

std::string
QmitkWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSP_ID;
}

void QmitkWorkbenchAdvisor::PreStartup()
{
// Todo: define nice qt style sheets and make them work
//#ifndef Q_WS_X11
//  QFile file(":/org_mitk_gui_qt_application/pagefold.qss");
//  file.open(QFile::ReadOnly);
//  QString styleSheet = QLatin1String(file.readAll());
//
//  qApp->setStyleSheet(styleSheet);
// 
//#endif
}
