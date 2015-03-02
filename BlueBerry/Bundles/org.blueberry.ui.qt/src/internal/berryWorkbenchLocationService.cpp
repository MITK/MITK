/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
