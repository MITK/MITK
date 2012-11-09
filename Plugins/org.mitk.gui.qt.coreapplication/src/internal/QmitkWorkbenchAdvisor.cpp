/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "QmitkWorkbenchAdvisor.h"

#include <QApplication>

#include "QmitkWorkbenchWindowAdvisor.h"

const std::string QmitkWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "org.mitk.coreapp.defaultperspective";

void
QmitkWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);
}

berry::WorkbenchWindowAdvisor*
QmitkWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  return new QmitkWorkbenchWindowAdvisor(configurer);
}

std::string QmitkWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSPECTIVE_ID;
}

