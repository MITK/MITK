/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFlowApplication.h"

#include <berryPlatformUI.h>

#include "QmitkFlowApplicationWorkbenchAdvisor.h"

QmitkFlowApplication::QmitkFlowApplication()
{

}

QVariant QmitkFlowApplication::Start(berry::IApplicationContext* /*context*/)
{
  QScopedPointer<berry::Display> display(berry::PlatformUI::CreateDisplay());

  QScopedPointer<QmitkFlowApplicationWorkbenchAdvisor> wbAdvisor(new QmitkFlowApplicationWorkbenchAdvisor());
  int code = berry::PlatformUI::CreateAndRunWorkbench(display.data(), wbAdvisor.data());

  // exit the application with an appropriate return code
  return code == berry::PlatformUI::RETURN_RESTART
              ? EXIT_RESTART : EXIT_OK;
}

void QmitkFlowApplication::Stop()
{

}
