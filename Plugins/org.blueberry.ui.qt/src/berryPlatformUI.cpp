/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPlatformUI.h"

#include "berryPlatform.h"
#include "berryIConfigurationElement.h"

#include "internal/berryWorkbench.h"

#include "berryUIException.h"

#include <vector>

namespace berry {

QString PlatformUI::PLUGIN_ID() { static QString str = "org.blueberry.ui"; return str; }
const QString PlatformUI::XP_WORKBENCH = PlatformUI::PLUGIN_ID() + ".workbench";
const QString PlatformUI::XP_VIEWS = PlatformUI::PLUGIN_ID() + ".views";

const int PlatformUI::RETURN_OK = 0;
const int PlatformUI::RETURN_RESTART = 1;
const int PlatformUI::RETURN_UNSTARTABLE = 2;
const int PlatformUI::RETURN_EMERGENCY_CLOSE = 3;

int
PlatformUI::CreateAndRunWorkbench(Display* display, WorkbenchAdvisor* advisor)
{
  return Workbench::CreateAndRunWorkbench(display, advisor);
}

Display* PlatformUI::CreateDisplay()
{
  return Workbench::CreateDisplay();
}

IWorkbench*
PlatformUI::GetWorkbench()
{
  if (Workbench::GetInstance() == nullptr)
  {
    // app forgot to call createAndRunWorkbench beforehand
    throw Poco::IllegalStateException("Workbench has not been created yet.");
  }
  return Workbench::GetInstance();
}

bool
PlatformUI::IsWorkbenchRunning()
{
  return Workbench::GetInstance() != nullptr
         && Workbench::GetInstance()->IsRunning();
}

TestableObject::Pointer
PlatformUI::GetTestableObject()
{
  return Workbench::GetWorkbenchTestable();
}

PlatformUI::PlatformUI()
{

}

}
