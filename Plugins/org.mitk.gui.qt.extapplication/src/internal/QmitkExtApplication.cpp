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

#include "QmitkExtApplication.h"

#include <berryPlatformUI.h>

#include "QmitkExtAppWorkbenchAdvisor.h"

QmitkExtApplication::QmitkExtApplication()
{

}

QVariant QmitkExtApplication::Start(berry::IApplicationContext* /*context*/)
{
  QScopedPointer<berry::Display> display(berry::PlatformUI::CreateDisplay());

  QScopedPointer<QmitkExtAppWorkbenchAdvisor> wbAdvisor(new QmitkExtAppWorkbenchAdvisor());
  int code = berry::PlatformUI::CreateAndRunWorkbench(display.data(), wbAdvisor.data());

  // exit the application with an appropriate return code
  return code == berry::PlatformUI::RETURN_RESTART
              ? EXIT_RESTART : EXIT_OK;
}

void QmitkExtApplication::Stop()
{

}
