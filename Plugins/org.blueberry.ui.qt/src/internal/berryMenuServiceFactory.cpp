/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    return nullptr;
  }

  IWorkbenchLocationService* wls = locator->GetService<IWorkbenchLocationService>();
  IWorkbench* const wb = wls->GetWorkbench();
  if (wb == nullptr)
  {
    return nullptr;
  }

  Object* parent = parentLocator->GetService(serviceInterface);
  if (parent == nullptr)
  {
    // we are registering the global services in the Workbench
    return nullptr;
  }
  IWorkbenchWindow* const window = wls->GetWorkbenchWindow();
  return new SlaveMenuService(dynamic_cast<InternalMenuService*>(parent), locator,
                              dynamic_cast<WorkbenchWindow*>(window)->GetMenuRestrictions());
}

}
