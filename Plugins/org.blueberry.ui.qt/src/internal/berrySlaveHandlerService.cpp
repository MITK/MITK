/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySlaveHandlerService.h"

#include "berryIHandler.h"
#include "berryIHandlerActivation.h"
#include "berryISourceProvider.h"

#include "berryAndExpression.h"
#include "berryHandlerActivation.h"
#include "berryExecutionEvent.h"

namespace berry {

SlaveHandlerService::SlaveHandlerService(IHandlerService* parentHandlerService,
                    const SmartPointer<Expression>& defaultExpression)
  : defaultExpression(defaultExpression)
  , parent(parentHandlerService)
{
  if (parentHandlerService == nullptr)
  {
    throw ctkInvalidArgumentException("The parent handler service cannot be null");
  }
}

SmartPointer<IHandlerActivation> SlaveHandlerService::ActivateHandler(const SmartPointer<IHandlerActivation>& childActivation)
{
  const QString commandId = childActivation->GetCommandId();
  const IHandler::Pointer handler = childActivation->GetHandler();
  const Expression::Pointer childExpression = childActivation->GetExpression();
  Expression::Pointer expression = defaultExpression;
  if (childExpression.IsNotNull() && defaultExpression.IsNotNull())
  {
    const AndExpression::Pointer andExpression(new AndExpression());
    andExpression->Add(childExpression);
    andExpression->Add(defaultExpression);
    expression = andExpression;
  }
  else if (childExpression.IsNotNull())
  {
    expression = childExpression;
  }
  const int depth = childActivation->GetDepth() + 1;
  const IHandlerActivation::Pointer localActivation(
        new HandlerActivation(commandId, handler, expression, depth, this));

  return DoActivation(localActivation);
}

SmartPointer<IHandlerActivation> SlaveHandlerService::ActivateHandler(const QString& commandId,
                                                 const SmartPointer<IHandler>& handler)
{
  const IHandlerActivation::Pointer localActivation(
        new HandlerActivation(commandId, handler, defaultExpression, IHandlerActivation::ROOT_DEPTH, this));
  return DoActivation(localActivation);
}

SmartPointer<IHandlerActivation> SlaveHandlerService::ActivateHandler(const QString& commandId,
                                                 const SmartPointer<IHandler>& handler,
                                                 const SmartPointer<Expression>& expression)
{
  return ActivateHandler(commandId, handler, expression, false);
}

SmartPointer<IHandlerActivation> SlaveHandlerService::ActivateHandler(const QString& commandId,
                                                 const SmartPointer<IHandler>& handler,
                                                 const SmartPointer<Expression>& expression,
                                                 bool global)
{
  if (global)
  {
    const IHandlerActivation::Pointer activation = parent->ActivateHandler(
          commandId, handler, expression, global);
    parentActivations.insert(activation);
    return activation;
  }

  Expression::Pointer aExpression = defaultExpression;
  if (expression.IsNotNull() && defaultExpression.IsNotNull())
  {
    const AndExpression::Pointer andExpression(new AndExpression());
    andExpression->Add(expression);
    andExpression->Add(defaultExpression);
    aExpression = andExpression;
  }
  else if (expression.IsNotNull())
  {
    aExpression = expression;
  }
  const IHandlerActivation::Pointer localActivation(
        new HandlerActivation(commandId, handler, aExpression, IHandlerActivation::ROOT_DEPTH, this));
  return DoActivation(localActivation);
}

void SlaveHandlerService::AddSourceProvider(const SmartPointer<ISourceProvider>& provider)
{
  if (!fSourceProviders.contains(provider))
  {
    fSourceProviders.push_back(provider);
  }
  parent->AddSourceProvider(provider);
}

SmartPointer<const ExecutionEvent> SlaveHandlerService::CreateExecutionEvent(const SmartPointer<const Command>& command,
                                                        const SmartPointer<const UIElement>& event)
{
  return parent->CreateExecutionEvent(command, event);
}

SmartPointer<const ExecutionEvent> SlaveHandlerService::CreateExecutionEvent(
    const SmartPointer<const ParameterizedCommand>& command,
    const SmartPointer<const UIElement>& event)
{
  return parent->CreateExecutionEvent(command, event);
}

void SlaveHandlerService::DeactivateHandler(const SmartPointer<IHandlerActivation>& activation)
{
  IHandlerActivation::Pointer parentActivation;
  if (localActivationsToParentActivations.contains(activation))
  {
    parentActivation = localActivationsToParentActivations.take(activation);
  }
  else
  {
    parentActivation = activation;
  }

  if (parentActivation.IsNotNull())
  {
    parent->DeactivateHandler(parentActivation);
    parentActivations.remove(parentActivation);
  }
}

void SlaveHandlerService::DeactivateHandlers(const QList<SmartPointer<IHandlerActivation> >& activations)
{
  for (int i = 0; i < activations.size(); i++)
  {
    DeactivateHandler(activations[i]);
  }
}

void SlaveHandlerService::Dispose()
{
  parent->DeactivateHandlers(parentActivations.toList());
  parentActivations.clear();
  localActivationsToParentActivations.clear();

  // Remove any "resource", like listeners, that were associated
  // with this service.
  if (!fSourceProviders.isEmpty())
  {
    for (int i = 0; i < fSourceProviders.size(); i++)
    {
      RemoveSourceProvider(fSourceProviders[i]);
    }
    fSourceProviders.clear();
  }
}

Object::Pointer SlaveHandlerService::ExecuteCommand(const SmartPointer<ParameterizedCommand>& command,
                               const SmartPointer<const UIElement>& event)
{
  return parent->ExecuteCommand(command, event);
}

Object::Pointer SlaveHandlerService::ExecuteCommand(const QString& commandId,
                               const SmartPointer<const UIElement>& event)
{
  return parent->ExecuteCommand(commandId, event);
}

SmartPointer<IEvaluationContext> SlaveHandlerService::GetCurrentState() const
{
  return parent->GetCurrentState();
}

void SlaveHandlerService::ReadRegistry()
{
  parent->ReadRegistry();
}

void SlaveHandlerService::RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider)
{
  fSourceProviders.removeAll(provider);
  parent->RemoveSourceProvider(provider);
}

void SlaveHandlerService::SetHelpContextId(const SmartPointer<IHandler>& handler,
                      const QString& helpContextId)
{
  parent->SetHelpContextId(handler, helpContextId);
}

SmartPointer<Expression> SlaveHandlerService::GetDefaultExpression() const
{
  return defaultExpression;
}

SmartPointer<IEvaluationContext> SlaveHandlerService::CreateContextSnapshot(bool includeSelection)
{
  return parent->CreateContextSnapshot(includeSelection);
}

Object::Pointer SlaveHandlerService::ExecuteCommandInContext(const SmartPointer<ParameterizedCommand>& command,
                                        const SmartPointer<const UIElement>& event,
                                        const SmartPointer<IEvaluationContext>& context)
{
  return parent->ExecuteCommandInContext(command, event, context);
}

SmartPointer<IHandlerActivation> SlaveHandlerService::DoActivation(
    const SmartPointer<IHandlerActivation>& localActivation)
{
  const IHandlerActivation::Pointer parentActivation = parent->ActivateHandler(localActivation);
  parentActivations.insert(parentActivation);
  localActivationsToParentActivations.insert(localActivation, parentActivation.GetPointer());
  return localActivation;
}

}
