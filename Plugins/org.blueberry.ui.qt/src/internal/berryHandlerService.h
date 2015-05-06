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

#ifndef BERRYHANDLERSERVICE_H
#define BERRYHANDLERSERVICE_H

#include "berryIHandlerService.h"

namespace berry {

struct ICommandService;
struct IEvaluationService;
struct IServiceLocator;

class HandlerAuthority;
class HandlerPersistence;

/**
 * <p>
 * Provides services related to activating and deactivating handlers within the
 * workbench.
 * </p>
 */
class HandlerService : public IHandlerService
{

private:

  /**
   * The command service for this handler service. This value is never
   * <code>null</code>.
   */
  ICommandService* const commandService;

  /**
   * The central authority for determining which handler we should use.
   */
  QScopedPointer<HandlerAuthority> handlerAuthority;

  /**
   * The class providing persistence for this service.
   */
  QScopedPointer<HandlerPersistence> handlerPersistence;

public:

  /**
   * Constructs a new instance of <code>CommandService</code> using a
   * command manager.
   *
   * @param commandService
   *            The command service to use; must not be <code>null</code>.
   * @param evaluationService
   *            The evaluation service to use; must not be <code>null</code>.
   * @param locator
   *            an appropriate service locator
   */
  HandlerService(ICommandService* commandService,
                 IEvaluationService* evaluationService,
                 IServiceLocator* locator);

  SmartPointer<IHandlerActivation> ActivateHandler(
      const SmartPointer<IHandlerActivation>& childActivation);

  SmartPointer<IHandlerActivation> ActivateHandler(const QString& commandId,
                                                   const SmartPointer<IHandler>& handler);

  SmartPointer<IHandlerActivation> ActivateHandler(const QString& commandId,
                                                   const SmartPointer<IHandler>& handler,
                                                   const SmartPointer<Expression>& expression);

  SmartPointer<IHandlerActivation> ActivateHandler(const QString& commandId,
                                                   const SmartPointer<IHandler>& handler,
                                                   const SmartPointer<Expression>& expression,
                                                   bool global);

  void AddSourceProvider(const SmartPointer<ISourceProvider>& provider);

  SmartPointer<const ExecutionEvent> CreateExecutionEvent(const SmartPointer<const Command>& command,
                                                    const SmartPointer<const UIElement>& trigger);

  SmartPointer<const ExecutionEvent> CreateExecutionEvent(
      const SmartPointer<const ParameterizedCommand>& command,
      const SmartPointer<const UIElement>& trigger);

  void DeactivateHandler(const SmartPointer<IHandlerActivation>& activation);

  void DeactivateHandlers(const QList<SmartPointer<IHandlerActivation> >& activations);

  void Dispose();

  Object::Pointer ExecuteCommand(const SmartPointer<ParameterizedCommand>& command,
                                 const SmartPointer<const UIElement>& trigger);

  Object::Pointer ExecuteCommand(const QString& commandId,
                                 const SmartPointer<const UIElement>& trigger);

  Object::Pointer ExecuteCommandInContext(const SmartPointer<ParameterizedCommand>& command,
                                          const SmartPointer<const UIElement>& trigger,
                                          const SmartPointer<IEvaluationContext>& context);

  SmartPointer<IEvaluationContext> GetCurrentState() const;

  void ReadRegistry();

  void RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider);

  void SetHelpContextId(const SmartPointer<IHandler>& handler,
                        const QString& helpContextId);

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

  /**
   * Currently this is a an internal method to help locate a handler.
   * <p>
   * DO NOT CALL THIS METHOD.
   * </p>
   *
   * @param commandId
   *            the command id to check
   * @param context
   *            the context to use for activations
   * @return the correct IHandler or <code>null</code>
   * @since 3.3
   */
  SmartPointer<IHandler> FindHandler(const QString& commandId,
                                     IEvaluationContext* context);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.handlers.IHandlerService#createContextSnapshot(boolean)
   */
  SmartPointer<IEvaluationContext> CreateContextSnapshot(bool includeSelection);

  Object::Pointer ExecuteCommandInContext(
      const SmartPointer<ParameterizedCommand>& command,
      const SmartPointer<const UIElement>& trigger,
      IEvaluationContext* context);

  /**
   * @return Returns the handlerPersistence.
   */
  HandlerPersistence* GetHandlerPersistence();
};

}

#endif // BERRYHANDLERSERVICE_H
