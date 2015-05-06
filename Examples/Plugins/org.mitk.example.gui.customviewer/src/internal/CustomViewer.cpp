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

#include "CustomViewer.h"
#include "CustomViewerWorkbenchAdvisor.h"

#include <berryPlatformUI.h>


CustomViewer::CustomViewer()
{
}

CustomViewer::~CustomViewer()
{
}

QVariant CustomViewer::Start(berry::IApplicationContext* /*context*/)
{
  berry::Display* display = berry::PlatformUI::CreateDisplay();

  CustomViewerWorkbenchAdvisor* wbAdvisor = new CustomViewerWorkbenchAdvisor;
  int code = berry::PlatformUI::CreateAndRunWorkbench(display, wbAdvisor);

  // exit the application with an appropriate return code
  return code == berry::PlatformUI::RETURN_RESTART
              ? EXIT_RESTART : EXIT_OK;
}

void CustomViewer::Stop()
{
}
