/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    ICommandService* commands = locator->GetService<ICommandService>();
    IEvaluationService* evals = locator->GetService<IEvaluationService>();
    auto   handlerService = new HandlerService(commands, evals, locator);
    handlerService->Register();
    handlerService->ReadRegistry();
    handlerService->UnRegister(false);
    return handlerService;
  }

  return nullptr;

  IWorkbenchWindow* const window = wls->GetWorkbenchWindow();
  IWorkbenchPartSite* const site = wls->GetPartSite();
  if (site == nullptr)
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
