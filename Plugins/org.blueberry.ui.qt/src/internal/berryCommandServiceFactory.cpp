/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandServiceFactory.h"

#include "berryICommandService.h"
#include "berryIServiceLocator.h"
#include "berryIServiceScopes.h"
#include "berryIWorkbenchLocationService.h"
#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPartSite.h"

#include "berrySlaveCommandService.h"

namespace berry {

Object* CommandServiceFactory::Create(const QString& serviceInterface, IServiceLocator* parentLocator,
                                      IServiceLocator* locator) const
{
  if (serviceInterface != qobject_interface_iid<ICommandService*>())
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
  IWorkbenchPartSite* const site = wls->GetPartSite();

  ICommandService* parentService = dynamic_cast<ICommandService*>(parent);

  if (site == nullptr)
  {
    return new SlaveCommandService(parentService,
                                   IServiceScopes::WINDOW_SCOPE, Object::Pointer(window));
  }

  return new SlaveCommandService(parentService,
                                 IServiceScopes::PARTSITE_SCOPE, Object::Pointer(site));
}

}
