/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWorkbenchLocationService.h"

namespace berry {

WorkbenchLocationService::WorkbenchLocationService(const QString& serviceScope, IWorkbench* workbench,
                                                   IWorkbenchWindow* window, IWorkbenchPartSite* partSite,
                                                   int level)
  : partSite(partSite)
  , serviceScope(serviceScope)
  , workbench(workbench)
  , window(window)
  , level(level)
{
}
IWorkbenchPartSite* WorkbenchLocationService::GetPartSite() const
{
  return partSite;
}

QString WorkbenchLocationService::GetServiceScope() const
{
  return serviceScope;
}

IWorkbench* WorkbenchLocationService::GetWorkbench() const
{
  return workbench;
}

IWorkbenchWindow* WorkbenchLocationService::GetWorkbenchWindow() const
{
  return window;
}

int WorkbenchLocationService::GetServiceLevel() const
{
  return level;
}

}
