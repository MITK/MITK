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

#include "berryMenuServiceFactory.h"

#include "berryIEvaluationReference.h"
#include "berryIWorkbenchLocationService.h"
#include "berryIWorkbenchWindow.h"

#include "berrySlaveMenuService.h"
#include "berryWorkbenchWindow.h"

namespace berry {

Object* MenuServiceFactory::Create(const QString& serviceInterface, IServiceLocator* parentLocator,
                                   IServiceLocator* locator) const
{
  if (serviceInterface != qobject_interface_iid<IMenuService*>())
  {
    return NULL;
  }

  IWorkbenchLocationService* wls = locator->GetService<IWorkbenchLocationService>();
  IWorkbench* const wb = wls->GetWorkbench();
  if (wb == NULL)
  {
    return NULL;
  }

  Object* parent = parentLocator->GetService(serviceInterface);
  if (parent == NULL)
  {
    // we are registering the global services in the Workbench
    return NULL;
  }
  IWorkbenchWindow* const window = wls->GetWorkbenchWindow();
  return new SlaveMenuService(dynamic_cast<InternalMenuService*>(parent), locator,
                              dynamic_cast<WorkbenchWindow*>(window)->GetMenuRestrictions());
}

}
