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
  IWorkbenchPartSite* const site = wls->GetPartSite();

  ICommandService* parentService = dynamic_cast<ICommandService*>(parent);

  if (site == NULL)
  {
    return new SlaveCommandService(parentService,
                                   IServiceScopes::WINDOW_SCOPE, Object::Pointer(window));
  }

  return new SlaveCommandService(parentService,
                                 IServiceScopes::PARTSITE_SCOPE, Object::Pointer(site));
}

}
