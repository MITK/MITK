/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef CHERRYIHANDLERSERVICE_H_
#define CHERRYIHANDLERSERVICE_H_

#include "../services/cherryIServiceWithSources.h"

#include "../common/cherryCommandExceptions.h"

namespace cherry {

class Command;
class ExecutionEvent;
class ParameterizedCommand;

namespace GuiTk { class Event; }

/**
 * <p>
 * Provides services related to activating and deactivating handlers within the
 * workbench.
 * </p>
 * <p>
 * This service can be acquired from your service locator:
 * <pre>
 *  IHandlerService service = (IHandlerService) getSite().getService(IHandlerService.class);
 * </pre>
 * <ul>
 * <li>This service is available globally.</li>
 * </ul>
 * </p>

 * @noimplement This interface is not intended to be implemented by clients.
 * @noextend This interface is not intended to be extended by clients.
 *
 * @since 3.1
 */
struct CHERRY_UI IHandlerService : public IServiceWithSources {

  cherryInterfaceMacro(IHandlerService, cherry)

  /**
   * <p>
   * Activates the given handler from a child service. This is used by slave
   * and nested services to promote handler activations up to the root. By
   * using this method, it is possible for handlers coming from a more nested
   * component to override the nested component.
   * </p>
   *
   * @param activation
   *            The activation that is local to the child service; must not be
   *            <code>null</code>.
   * @return A token which can be used to later cancel the activation. Only
   *         someone with access to this token can cancel the activation. The
   *         activation will automatically be cancelled if the service locator
   *         context from which this service was retrieved is destroyed. This
   *         activation is local to this service (i.e., it is not the
   *         activation that is passed as a parameter).
   * @since 3.2
   */
  virtual SmartPointer<IHandlerActivation> ActivateHandler(SmartPointer<IHandlerActivation> activation) = 0;

  /**
   * <p>
   * Activates the given handler within the context of this service. If this
   * service was retrieved from the workbench, then this handler will be
   * active globally. If the service was retrieved from a nested component,
   * then the handler will only be active within that component.
   * </p>
   * <p>
   * Also, it is guaranteed that the handlers submitted through a particular
   * service will be cleaned up when that services is destroyed. So, for
   * example, a service retrieved from a <code>IWorkbenchPartSite</code>
   * would deactivate all of its handlers when the site is destroyed.
   * </p>
   *
   * @param commandId
   *            The identifier for the command which this handler handles;
   *            must not be <code>null</code>.
   * @param handler
   *            The handler to activate; must not be <code>null</code>.
   * @return A token which can be used to later cancel the activation. Only
   *         someone with access to this token can cancel the activation. The
   *         activation will automatically be cancelled if the context from
   *         which this service was retrieved is destroyed.
   */
  virtual SmartPointer<IHandlerActivation> ActivateHandler(
      const std::string& commandId, SmartPointer<IHandler> handler) = 0;

  /**
   * <p>
   * Activates the given handler within the context of this service. The
   * handler becomes active when <code>expression</code> evaluates to
   * <code>true</code>. This is the same as calling
   * {@link #activateHandler(String, IHandler, Expression, boolean)} with
   * global==false.
   * </p>
   * <p>
   * Also, it is guaranteed that the handlers submitted through a particular
   * service will be cleaned up when that service is destroyed. So, for
   * example, a service retrieved from a <code>IWorkbenchPartSite</code>
   * would deactivate all of its handlers when the site is destroyed.
   * </p>
   *
   * @param commandId
   *            The identifier for the command which this handler handles;
   *            must not be <code>null</code>.
   * @param handler
   *            The handler to activate; must not be <code>null</code>.
   * @param expression
   *            This expression must evaluate to <code>true</code> before
   *            this handler will really become active. The expression may be
   *            <code>null</code> if the handler should always be active.
   * @return A token which can be used to later cancel the activation. Only
   *         someone with access to this token can cancel the activation. The
   *         activation will automatically be cancelled if the context from
   *         which this service was retrieved is destroyed.
   *
   * @see org.eclipse.ui.ISources
   * @since 3.2
   */
  virtual SmartPointer<IHandlerActivation> ActivateHandler(
      const std::string& commandId,
      SmartPointer<IHandler> handler, SmartPointer<Expression> expression) = 0;

  /**
   * <p>
   * Activates the given handler within the context of this service. The
   * handler becomes active when <code>expression</code> evaluates to
   * <code>true</code>. if global==<code>false</code>, then this
   * handler service must also be the active service to active the handler.
   * For example, the handler service on a part is active when that part is
   * active.
   * </p>
   * <p>
   * Also, it is guaranteed that the handlers submitted through a particular
   * service will be cleaned up when that services is destroyed. So, for
   * example, a service retrieved from a <code>IWorkbenchPartSite</code>
   * would deactivate all of its handlers when the site is destroyed.
   * </p>
   *
   * @param commandId
   *            The identifier for the command which this handler handles;
   *            must not be <code>null</code>.
   * @param handler
   *            The handler to activate; must not be <code>null</code>.
   * @param expression
   *            This expression must evaluate to <code>true</code> before
   *            this handler will really become active. The expression may be
   *            <code>null</code> if the handler should always be active.
   * @param global
   *            Indicates that the handler should be activated irrespectively
   *            of whether the corresponding workbench component (e.g.,
   *            window, part, etc.) is active.
   * @return A token which can be used to later cancel the activation. Only
   *         someone with access to this token can cancel the activation. The
   *         activation will automatically be cancelled if the context from
   *         which this service was retrieved is destroyed.
   *
   * @see org.eclipse.ui.ISources
   * @since 3.2
   */
  virtual SmartPointer<IHandlerActivation> ActivateHandler(const std::string& commandId,
      SmartPointer<IHandler> handler, SmartPointer<Expression> expression, bool global) = 0;

  /**
   * <p>
   * Activates the given handler within the context of this service. The
   * handler becomes active when <code>expression</code> evaluates to
   * <code>true</code>.
   * </p>
   * <p>
   * Also, it is guaranteed that the handlers submitted through a particular
   * service will be cleaned up when that services is destroyed. So, for
   * example, a service retrieved from a <code>IWorkbenchPartSite</code>
   * would deactivate all of its handlers when the site is destroyed.
   * </p>
   *
   * @param commandId
   *            The identifier for the command which this handler handles;
   *            must not be <code>null</code>.
   * @param handler
   *            The handler to activate; must not be <code>null</code>.
   * @param expression
   *            This expression must evaluate to <code>true</code> before
   *            this handler will really become active. The expression may be
   *            <code>null</code> if the handler should always be active.
   * @param sourcePriorities
   *            The source priorities for the expression.
   * @return A token which can be used to later cancel the activation. Only
   *         someone with access to this token can cancel the activation. The
   *         activation will automatically be cancelled if the context from
   *         which this service was retrieved is destroyed.
   *
   * @see org.eclipse.ui.ISources
   * @deprecated Use
   *             {@link IHandlerService#activateHandler(String, IHandler, Expression)}
   *             instead.
   */
  virtual SmartPointer<IHandlerActivation> ActivateHandler(const std::string& commandId,
      SmartPointer<IHandler> handler, SmartPointer<Expression> expression,
      int sourcePriorities) = 0;

  /**
   * Creates an execution event based on an SWT event. This execution event
   * can then be passed to a command for execution.
   *
   * @param command
   *            The command for which an execution event should be created;
   *            must not be <code>null</code>.
   * @param event
   *            The SWT event triggering the command execution; may be
   *            <code>null</code>.
   * @return An execution event suitable for calling
   *         {@link Command#executeWithChecks(ExecutionEvent)}.
   * @since 3.2
   * @see Command#executeWithChecks(ExecutionEvent)
   */
  virtual SmartPointer<const ExecutionEvent> CreateExecutionEvent(
      SmartPointer<const Command> command, SmartPointer<const GuiTk::Event> event) = 0;

  /**
   * Creates a parameterized execution event based on an SWT event and a
   * parameterized command. This execution event can then be passed to a
   * command for execution.
   *
   * @param command
   *            The parameterized command for which an execution event should
   *            be created; must not be <code>null</code>.
   * @param event
   *            The SWT event triggering the command execution; may be
   *            <code>null</code>.
   * @return An execution event suitable for calling
   *         {@link Command#executeWithChecks(ExecutionEvent)}.
   * @since 3.2
   * @see ParameterizedCommand#getCommand()
   * @see Command#executeWithChecks(ExecutionEvent)
   */
  virtual SmartPointer<const ExecutionEvent> CreateExecutionEvent(
      SmartPointer<const ParameterizedCommand> command,
      SmartPointer<const GuiTk::Event> event) = 0;

  /**
   * Deactivates the given handler within the context of this service. If the
   * handler was activated with a different service, then it must be
   * deactivated from that service instead. It is only possible to retract a
   * handler activation with this method. That is, you must have the same
   * <code>IHandlerActivation</code> used to activate the handler.
   *
   * @param activation
   *            The token that was returned from a call to
   *            <code>activateHandler</code>; must not be <code>null</code>.
   */
  virtual void DeactivateHandler(SmartPointer<IHandlerActivation> activation) = 0;

  /**
   * Deactivates the given handlers within the context of this service. If the
   * handler was activated with a different service, then it must be
   * deactivated from that service instead. It is only possible to retract a
   * handler activation with this method. That is, you must have the same
   * <code>IHandlerActivation</code> used to activate the handler.
   *
   * @param activations
   *            The tokens that were returned from a call to
   *            <code>activateHandler</code>. This collection must only
   *            contain instances of <code>IHandlerActivation</code>. The
   *            collection must not be <code>null</code>.
   */
  virtual void DeactivateHandlers(
      const std::vector<SmartPointer<IHandlerActivation> >& activations) = 0;

  /**
   * Executes the command with the given identifier and no parameters.
   *
   * @param commandId
   *            The identifier of the command to execute; must not be
   *            <code>null</code>.
   * @param event
   *            The SWT event triggering the command execution; may be
   *            <code>null</code>.
   * @return The return value from the execution; may be <code>null</code>.
   * @throws ExecutionException
   *             If the handler has problems executing this command.
   * @throws NotDefinedException
   *             If the command you are trying to execute is not defined.
   * @throws NotEnabledException
   *             If the command you are trying to execute is not enabled.
   * @throws NotHandledException
   *             If there is no handler.
   * @since 3.2
   * @see Command#executeWithChecks(ExecutionEvent)
   */
  virtual Object::Pointer ExecuteCommand(const std::string& commandId,
      SmartPointer<const GuiTk::Event> event)
      throw(ExecutionException, NotDefinedException,
      NotEnabledException, NotHandledException) = 0;

  /**
   * Executes the given parameterized command.
   *
   * @param command
   *            The parameterized command to be executed; must not be
   *            <code>null</code>.
   * @param event
   *            The SWT event triggering the command execution; may be
   *            <code>null</code>.
   * @return The return value from the execution; may be <code>null</code>.
   * @throws ExecutionException
   *             If the handler has problems executing this command.
   * @throws NotDefinedException
   *             If the command you are trying to execute is not defined.
   * @throws NotEnabledException
   *             If the command you are trying to execute is not enabled.
   * @throws NotHandledException
   *             If there is no handler.
   * @since 3.2
   * @see Command#executeWithChecks(ExecutionEvent)
   */
  virtual Object::Pointer ExecuteCommand(
      SmartPointer<ParameterizedCommand> command,
      SmartPointer<const GuiTk::Event> event)
      throw(ExecutionException, NotDefinedException,
      NotEnabledException, NotHandledException) = 0;

  /**
   * Executes the given parameterized command in the provided context. It
   * takes care of finding the correct active handler given the context, calls
   * {@link IHandler2#setEnabled(Object)} to update the enabled state if
   * supported, and executes with that handler.
   *
   * @param command
   *            The parameterized command to be executed; must not be
   *            <code>null</code>.
   * @param event
   *            The SWT event triggering the command execution; may be
   *            <code>null</code>.
   * @param context
   *            the evaluation context to run against. Must not be
   *            <code>null</code>
   * @return The return value from the execution; may be <code>null</code>.
   * @throws ExecutionException
   *             If the handler has problems executing this command.
   * @throws NotDefinedException
   *             If the command you are trying to execute is not defined.
   * @throws NotEnabledException
   *             If the command you are trying to execute is not enabled.
   * @throws NotHandledException
   *             If there is no handler.
   * @since 3.4
   * @see Command#executeWithChecks(ExecutionEvent)
   * @see #createContextSnapshot(boolean)
   */
  virtual Object::Pointer ExecuteCommandInContext(
      SmartPointer<ParameterizedCommand> command,
      SmartPointer<const GuiTk::Event> event,
      SmartPointer<IEvaluationContext> context)
    throw(ExecutionException,
      NotDefinedException, NotEnabledException, NotHandledException) = 0;

  /**
   * This method creates a copy of the application context returned by
   * {@link #getCurrentState()}.
   *
   * @param includeSelection
   *            if <code>true</code>, include the default variable and
   *            selection variables
   * @return an context filled with the current set of variables. If selection
   *         is not included, the default variable is an empty collection
   * @since 3.4
   */
  virtual SmartPointer<IEvaluationContext> CreateContextSnapshot(bool includeSelection) = 0;

  /**
   * Returns an evaluation context representing the current state of the
   * world. This is equivalent to the application context required by
   * {@link ExecutionEvent}.
   *
   * @return the current state of the application; never <code>null</code>.
   * @see ParameterizedCommand#executeWithChecks(Object, Object)
   * @see ExecutionEvent#ExecutionEvent(Command, java.util.Map, Object,
   *      Object)
   * @see org.eclipse.ui.services.IEvaluationService
   */
  virtual SmartPointer<IEvaluationContext> GetCurrentState() const = 0;

  /**
   * <p>
   * Reads the handler information from the registry. This will overwrite any
   * of the existing information in the handler service. This method is
   * intended to be called during start-up. When this method completes, this
   * handler service will reflect the current state of the registry.
   * </p>
   */
  virtual void ReadRegistry() = 0;

  /**
   * Sets the help context identifier to associate with a particular handler.
   *
   * @param handler
   *            The handler with which to register a help context identifier;
   *            must not be <code>null</code>.
   * @param helpContextId
   *            The help context identifier to register; may be
   *            <code>null</code> if the help context identifier should be
   *            removed.
   * @since 3.2
   */
  virtual void SetHelpContextId(SmartPointer<IHandler> handler, const std::string& helpContextId) = 0;
};

}

#endif /* CHERRYIHANDLERSERVICE_H_ */
