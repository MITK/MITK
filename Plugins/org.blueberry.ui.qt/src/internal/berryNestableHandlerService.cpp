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

#include "berryNestableHandlerService.h"

#include "berryExpression.h"
#include "berryIHandlerActivation.h"
#include "berryISourceProvider.h"

namespace berry {

NestableHandlerService::NestableHandlerService(IHandlerService* parentHandlerService,
                       const SmartPointer<Expression>& defaultExpression)
  : SlaveHandlerService(parentHandlerService, defaultExpression)
  , active(false)
{
}

void NestableHandlerService::Activate()
{
  if (active)
  {
    return;
  }

  QList<IHandlerActivation::Pointer> localActivations = localActivationsToParentActivations.keys();
  for (int i = 0; i < localActivations.size(); i++)
  {
    // Ignore activations that have been cleared since the copy
    // was made.
    if (localActivationsToParentActivations.contains(localActivations[i]))
    {
      SlaveHandlerService::DoActivation(localActivations[i]);
    }
  }
  active = true;
}

void NestableHandlerService::Deactivate()
{
  if (!active)
  {
    return;
  }

  DeactivateHandlers(parentActivations.toList());
  parentActivations.clear();

  QList<IHandlerActivation::Pointer> localActivations = localActivationsToParentActivations.keys();
  for (int i = 0; i < localActivations.size(); i++)
  {
    if (localActivationsToParentActivations.contains(localActivations[i]))
    {
      localActivationsToParentActivations.insert(localActivations[i], NULL);
    }
  }

  active = false;
}

SmartPointer<IHandlerActivation> NestableHandlerService::DoActivation(
    const SmartPointer<IHandlerActivation>& localActivation)
{
  if (active)
  {
    return SlaveHandlerService::DoActivation(localActivation);
  }
  localActivationsToParentActivations.insert(localActivation, NULL);
  return localActivation;
}

}
