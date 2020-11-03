/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHandlerService.h"

#include "berryHandlerAuthority.h"
#include "berryHandlerPersistence.h"
#include "berryIHandler.h"
#include "berryHandlerActivation.h"
#include "berryExpression.h"
#include "berryExecutionEvent.h"
#include "berryParameterizedCommand.h"
#include "berryICommandService.h"
#include "berryUIElement.h"

namespace berry {

HandlerService::HandlerService(ICommandService* commandService,
                               IEvaluationService* evaluationService,
                               IServiceLocator* locator)
  : commandService(commandService)
  , handlerAuthority(new HandlerAuthority(commandService, locator))
  , handlerPersistence(new HandlerPersistence(this, evaluationService))
{
  if (commandService == nullptr)
  {
    throw ctkInvalidArgumentException("A handler service requires a command service");
  }
}

SmartPointer<IHandlerActivation> HandlerService::ActivateHandler(
    const SmartPointer<IHandlerActivation>& childActivation)
{
  const QString commandId = childActivation->GetCommandId();
  const IHandler::Pointer handler = childActivation->GetHandler();
  const Expression::Pointer expression = childActivation->GetExpression();
  const int depth = childActivation->GetDepth() + 1;
  const IHandlerActivation::Pointer localActivation(
        new HandlerActivation(commandId, handler, expression, depth, this));
  handlerAuthority->ActivateHandler(localActivation);
  return localActivation;
}

SmartPointer<IHandlerActivation> HandlerService::ActivateHandler(const QString& commandId,
                                                 const SmartPointer<IHandler>& handler)
{
  return ActivateHandler(commandId, handler, Expression::Pointer(nullptr));
}

SmartPointer<IHandlerActivation> HandlerService::ActivateHandler(const QString& commandId,
                                                 const SmartPointer<IHandler>& handler,
                                                 const SmartPointer<Expression>& expression)
{
  return ActivateHandler(commandId, handler, expression, false);
}

SmartPointer<IHandlerActivation> HandlerService::ActivateHandler(const QString& commandId,
                                                 const SmartPointer<IHandler>& handler,
                                                 const SmartPointer<Expression>& expression,
                                                 bool /*global*/)
{
  const IHandlerActivation::Pointer activation(
        new HandlerActivation(commandId, handler, expression, IHandlerActivation::ROOT_DEPTH, this));
  handlerAuthority->ActivateHandler(activation);
  return activation;
}

void HandlerService::AddSourceProvider(const SmartPointer<ISourceProvider>& /*provider*/)
{
  // no-op
}

SmartPointer<const ExecutionEvent> HandlerService::CreateExecutionEvent(const SmartPointer<const Command>& command,
                                                                  const SmartPointer<const UIElement>& trigger)
{
  ExecutionEvent::Pointer event(new ExecutionEvent(command, ExecutionEvent::ParameterMap(), trigger,
                                                   Object::Pointer(GetCurrentState())));
  return event;
}

SmartPointer<const ExecutionEvent> HandlerService::CreateExecutionEvent(
    const SmartPointer<const ParameterizedCommand>& command,
    const SmartPointer<const UIElement>& trigger)
{
  ExecutionEvent::Pointer event(
        new ExecutionEvent(command->GetCommand(), command->GetParameterMap(), trigger,
                           Object::Pointer(GetCurrentState())));
  return event;
}

void HandlerService::DeactivateHandler(const SmartPointer<IHandlerActivation>& activation)
{
  if (activation->GetHandlerService() == this)
  {
    handlerAuthority->DeactivateHandler(activation);
  }
}

void HandlerService::DeactivateHandlers(const QList<SmartPointer<IHandlerActivation> >& activations)
{
  foreach (const IHandlerActivation::Pointer activation, activations)
  {
    DeactivateHandler(activation);
  }
}

void HandlerService::Dispose()
{
  handlerPersistence.reset();
  handlerAuthority.reset();
}

Object::Pointer HandlerService::ExecuteCommand(const SmartPointer<ParameterizedCommand>& command,
                                               const SmartPointer<const UIElement>& trigger)
{
  return command->ExecuteWithChecks(trigger, Object::Pointer(GetCurrentState()));
}

Object::Pointer HandlerService::ExecuteCommand(const QString& commandId,
                                               const SmartPointer<const UIElement>& trigger)
{
  const Command::Pointer command = commandService->GetCommand(commandId);
  const ExecutionEvent::Pointer event(
        new ExecutionEvent(command, ExecutionEvent::ParameterMap(), trigger,
                           Object::Pointer(GetCurrentState())));
  return command->ExecuteWithChecks(event);
}

Object::Pointer HandlerService::ExecuteCommandInContext(const SmartPointer<ParameterizedCommand>& command,
                                                        const SmartPointer<const UIElement>& trigger,
                                                        const SmartPointer<IEvaluationContext>& context)
{
  IHandler::Pointer oldHandler = command->GetCommand()->GetHandler();

  IHandler::Pointer handler = FindHandler(command->GetId(), context.GetPointer());
  handler->SetEnabled(context);

  struct RestoreHandlerState {
    RestoreHandlerState(HandlerService* hs, ParameterizedCommand* cmd, IHandler* handler, IHandler* oldHandler)
      : hs(hs), cmd(cmd), handler(handler), oldHandler(oldHandler) {}
    ~RestoreHandlerState() {
      cmd->GetCommand()->SetHandler(IHandler::Pointer(oldHandler));
      handler->SetEnabled(hs->GetCurrentState());
    }
  private:
    HandlerService* hs;
    ParameterizedCommand* cmd;
    IHandler* handler;
    IHandler* oldHandler;
  };

  RestoreHandlerState restorer(this, command.GetPointer(), handler.GetPointer(), oldHandler.GetPointer());
  command->GetCommand()->SetHandler(handler);
  return command->ExecuteWithChecks(trigger, context);
 }

SmartPointer<IEvaluationContext> HandlerService::GetCurrentState() const
{
  return handlerAuthority->GetCurrentState();
}

void HandlerService::ReadRegistry()
{
  handlerPersistence->Read();
}

void HandlerService::RemoveSourceProvider(const SmartPointer<ISourceProvider>& /*provider*/)
{
  // this is a no-op
}

void HandlerService::SetHelpContextId(const SmartPointer<IHandler>& handler,
                      const QString& helpContextId)
{
  commandService->SetHelpContextId(handler, helpContextId);
}

/*
 * <p>
 * Bug 95792. A mechanism by which the key binding architecture can force an
 * update of the handlers (based on the active shell) before trying to
 * execute a command. This mechanism is required for GTK+ only.
 * </p>
 * <p>
 * DO NOT CALL THIS METHOD.
 * </p>
 */
//  void UpdateShellKludge() {
//    handlerAuthority.updateShellKludge();
//  }

/*
 * <p>
 * Bug 95792. A mechanism by which the key binding architecture can force an
 * update of the handlers (based on the active shell) before trying to
 * execute a command. This mechanism is required for GTK+ only.
 * </p>
 * <p>
 * DO NOT CALL THIS METHOD.
 * </p>
 *
 * @param shell
 *            The shell that should be considered active; must not be
 *            <code>null</code>.
 */
//  void UpdateShellKludge(const SmartPointer<Shell>& shell)
//  {
//    handlerAuthority.updateShellKludge(shell);
//  }

SmartPointer<IHandler> HandlerService::FindHandler(const QString& commandId,
                                   IEvaluationContext* context)
{
  return handlerAuthority->FindHandler(commandId, context);
}

/*
 * (non-Javadoc)
 *
 * @see org.eclipse.ui.handlers.IHandlerService#createContextSnapshot(boolean)
 */
SmartPointer<IEvaluationContext> HandlerService::CreateContextSnapshot(bool includeSelection)
{
  return handlerAuthority->CreateContextSnapshot(includeSelection);
}

Object::Pointer HandlerService::ExecuteCommandInContext(
    const SmartPointer<ParameterizedCommand>& command,
    const SmartPointer<const UIElement>& trigger,
    IEvaluationContext* context)
{
  IHandler::Pointer oldHandler = command->GetCommand()->GetHandler();

  IHandler::Pointer handler = FindHandler(command->GetId(), context);
  handler->SetEnabled(Object::Pointer(context));

  struct RestoreHandler {
    RestoreHandler(HandlerService* hs, ParameterizedCommand* cmd, IHandler* handler, IHandler* oldHandler)
      : hs(hs), cmd(cmd), handler(handler), oldHandler(oldHandler)
    {}

    ~RestoreHandler()
    {
      cmd->GetCommand()->SetHandler(IHandler::Pointer(oldHandler));
      handler->SetEnabled(Object::Pointer(hs->GetCurrentState()));
    }

    HandlerService* const hs;
    ParameterizedCommand* const cmd;
    IHandler* const handler;
    IHandler* const oldHandler;
  };

  RestoreHandler restorer(this, command.GetPointer(), handler.GetPointer(), oldHandler.GetPointer());
  command->GetCommand()->SetHandler(handler);

  return command->ExecuteWithChecks(trigger, Object::Pointer(context));
}

HandlerPersistence* HandlerService::GetHandlerPersistence()
{
  return handlerPersistence.data();
}

}
