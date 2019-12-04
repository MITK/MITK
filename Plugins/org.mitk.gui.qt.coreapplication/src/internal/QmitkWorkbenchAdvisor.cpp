/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkWorkbenchAdvisor.h"

#include <QApplication>

#include "QmitkWorkbenchWindowAdvisor.h"

const QString QmitkWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "org.mitk.coreapp.defaultperspective";

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

QString QmitkWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSPECTIVE_ID;
}

