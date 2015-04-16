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

#include "berryHandlerServiceFactory.h"

#include "berryIHandlerService.h"
#include "berryIWorkbenchLocationService.h"
#include "berryIWorkbench.h"
#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPartSite.h"
#include "berryICommandService.h"
#include "berryIEvaluationService.h"

#include "berryExpression.h"
#include "berryHandlerService.h"
#include "berryActivePartExpression.h"
#include "berryWorkbenchWindowExpression.h"
#include "berrySlaveHandlerService.h"
#include "berryNestableHandlerService.h"

namespace berry {

Object* HandlerServiceFactory::Create(
    const QString& serviceInterface, IServiceLocator* parentLocator,
    IServiceLocator* locator) const
{
  if (serviceInterface != qobject_interface_iid<IHandlerService*>())
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
    ICommandService* commands = locator->GetService<ICommandService>();
    IEvaluationService* evals = locator->GetService<IEvaluationService>();
    HandlerService* handlerService = new HandlerService(commands, evals, locator);
    handlerService->Register();
    handlerService->ReadRegistry();
    handlerService->UnRegister(false);
    return handlerService;
  }

  return NULL;

  IWorkbenchWindow* const window = wls->GetWorkbenchWindow();
  IWorkbenchPartSite* const site = wls->GetPartSite();
  if (site == NULL)
  {
    Expression::Pointer exp(new WorkbenchWindowExpression(window));
    return new SlaveHandlerService(dynamic_cast<IHandlerService*>(parent), exp);
  }

  if (SlaveHandlerService* slaveParent = dynamic_cast<SlaveHandlerService*>(parent))
  {
    Expression::Pointer parentExp = slaveParent->GetDefaultExpression();
    if (parentExp.Cast<ActivePartExpression>())
    {
      return new NestableHandlerService(dynamic_cast<IHandlerService*>(parent), parentExp);
    }
  }

  Expression::Pointer exp(new ActivePartExpression(site->GetPart().GetPointer()));
  return new SlaveHandlerService(dynamic_cast<IHandlerService*>(parent), exp);
}

}
