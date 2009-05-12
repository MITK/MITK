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

#ifndef CHERRYCOMMAND_H_
#define CHERRYCOMMAND_H_

#include "cherryNamedHandleObjectWithState.h"
#include "cherryIParameter.h"
#include "cherryParameterType.h"

#include "cherryIExecutionListenerWithChecks.h"
#include "cherryICommandListener.h"
#include "cherryIHandlerListener.h"

#include <vector>

namespace cherry
{

class CommandCategory;
class ExecutionEvent;
struct IHandler;

/**
 * <p>
 * A command is an abstract representation for some semantic behaviour. It is
 * not the actual implementation of this behaviour, nor is it the visual
 * appearance of this behaviour in the user interface. Instead, it is a bridge
 * between the two.
 * </p>
 * <p>
 * The concept of a command is based on the command design pattern. The notable
 * difference is how the command delegates responsibility for execution. Rather
 * than allowing concrete subclasses, it uses a handler mechanism (see the
 * <code>handlers</code> extension point). This provides another level of
 * indirection.
 * </p>
 * <p>
 * A command will exist in two states: defined and undefined. A command is
 * defined if it is declared in the XML of a resolved plug-in. If the plug-in is
 * unloaded or the command is simply not declared, then it is undefined. Trying
 * to reference an undefined command will succeed, but trying to access any of
 * its functionality will fail with a <code>NotDefinedException</code>. If
 * you need to know when a command changes from defined to undefined (or vice
 * versa), then attach a command listener.
 * </p>
 * <p>
 * Commands are mutable and will change as their definition changes.
 * </p>
 *
 */
class CHERRY_COMMANDS Command: public NamedHandleObjectWithState
{ // implements Comparable {

public:

  cherryObjectMacro(Command)

  /**
   * This flag can be set to <code>true</code> if commands should print
   * information to <code>System.out</code> when executing.
   */
  static  bool DEBUG_COMMAND_EXECUTION;

  /**
   * This flag can be set to <code>true</code> if commands should print
   * information to <code>System.out</code> when changing handlers.
   */
  static bool DEBUG_HANDLERS;

  /**
   * This flag can be set to a particular command identifier if only that
   * command should print information to <code>System.out</code> when
   * changing handlers.
   */
  static std::string DEBUG_HANDLERS_COMMAND_ID;

private:

  /**
   * The category to which this command belongs. This value should not be
   * <code>null</code> unless the command is undefined.
   */
  SmartPointer<CommandCategory> category;

  /**
   * A collection of objects listening to the execution of this command.
   */
  IExecutionListenerWithChecks::Events executionEvents;
  //ListenerList executionListeners;

  /**
   * A collection of objects listening to changes of this command.
   */
  ICommandListener::Events commandEvents;

  /**
   * The handler currently associated with this command. This value may be
   * <code>null</code> if there is no handler currently.
   */
  SmartPointer<IHandler> handler;

  /**
   * The help context identifier for this command. This can be
   * <code>null</code> if there is no help currently associated with the
   * command.
   *
   * @since 3.2
   */
  std::string helpContextId;

  /**
   * The ordered array of parameters understood by this command. This value
   * may be <code>null</code> if there are no parameters, or if the command
   * is undefined. It may also be empty.
   */
  std::vector<SmartPointer<IParameter> > parameters;

  /**
   * The type of the return value of this command. This value may be
   * <code>null</code> if the command does not declare a return type.
   *
   * @since 3.2
   */
  SmartPointer<ParameterType> returnType;

  /**
   * Our command will listen to the active handler for enablement changes so
   * that they can be fired from the command itself.
   *
   * @since 3.3
   */
  SmartPointer<IHandlerListener> handlerListener;

protected:

  /**
   * Constructs a new instance of <code>Command</code> based on the given
   * identifier. When a command is first constructed, it is undefined.
   * Commands should only be constructed by the <code>CommandManager</code>
   * to ensure that the identifier remains unique.
   *
   * @param id
   *            The identifier for the command. This value must not be
   *            <code>null</code>, and must be unique amongst all commands.
   */
  Command(const std::string& id);

  friend class CommandManager;

public:

  /**
   * Adds a listener to this command that will be notified when this command's
   * state changes.
   *
   * @param commandListener
   *            The listener to be added; must not be <code>null</code>.
   */
  void AddCommandListener(const ICommandListener::Pointer commandListener);

  /**
   * Adds a listener to this command that will be notified when this command
   * is about to execute.
   *
   * @param executionListener
   *            The listener to be added; must not be <code>null</code>.
   */
  void AddExecutionListener(
      const IExecutionListener::Pointer executionListener);

  /**
   * <p>
   * Adds a state to this command. This will add this state to the active
   * handler, if the active handler is an instance of {@link IObjectWithState}.
   * </p>
   * <p>
   * A single instance of {@link State} cannot be registered with multiple
   * commands. Each command requires its own unique instance.
   * </p>
   *
   * @param id
   *            The identifier of the state to add; must not be
   *            <code>null</code>.
   * @param state
   *            The state to add; must not be <code>null</code>.
   * @since 3.2
   */
  void AddState(const std::string& id, const State::Pointer state);

  /**
   * Compares this command with another command by comparing each of its
   * non-transient attributes.
   *
   * @param object
   *            The object with which to compare; must be an instance of
   *            <code>Command</code>.
   * @return false if the object is
   *         equal to or greater than this command.
   */
  bool operator<(const Object* object) const;

  /**
   * <p>
   * Defines this command by giving it a name, and possibly a description as
   * well. The defined property automatically becomes <code>true</code>.
   * </p>
   * <p>
   * Notification is sent to all listeners that something has changed.
   * </p>
   *
   * @param name
   *            The name of this command; must not be <code>null</code>.
   * @param description
   *            The description for this command; may be <code>null</code>.
   * @param category
   *            The category for this command; must not be <code>null</code>.
   * @param parameters
   *            The parameters understood by this command. This value may be
   *            either <code>null</code> or empty if the command does not
   *            accept parameters.
   * @param returnType
   *            The type of value returned by this command. This value may be
   *            <code>null</code> if the command does not declare a return
   *            type.
   * @param helpContextId
   *            The identifier of the help context to associate with this
   *            command; may be <code>null</code> if this command does not
   *            have any help associated with it.
   * @since 3.2
   */
  void Define(const std::string& name, const std::string& description,
      const SmartPointer<CommandCategory> category,
      const std::vector<SmartPointer<IParameter> >& parameters = std::vector<SmartPointer<IParameter> >(),
      SmartPointer<ParameterType> returnType = SmartPointer<ParameterType>(0), const std::string& helpContextId = "");

  /**
   * Executes this command by delegating to the current handler, if any. If
   * the debugging flag is set, then this method prints information about
   * which handler is selected for performing this command. This does checks
   * to see if the command is enabled and defined. If it is not both enabled
   * and defined, then the execution listeners will be notified and an
   * exception thrown.
   *
   * @param event
   *            An event containing all the information about the current
   *            state of the application; must not be <code>null</code>.
   * @return The result of the execution; may be <code>null</code>. This
   *         result will be available to the client executing the command, and
   *         execution listeners.
   * @throws ExecutionException
   *             If the handler has problems executing this command.
   * @throws NotDefinedException
   *             If the command you are trying to execute is not defined.
   * @throws NotEnabledException
   *             If the command you are trying to execute is not enabled.
   * @throws NotHandledException
   *             If there is no handler.
   * @since 3.2
   */
  Object::Pointer ExecuteWithChecks(const SmartPointer<const ExecutionEvent> event);

private:

  /**
   * Notifies the listeners for this command that it has changed in some way.
   *
   * @param commandEvent
   *            The event to send to all of the listener; must not be
   *            <code>null</code>.
   */
  void FireCommandChanged(const SmartPointer<const CommandEvent> commandEvent);

  /**
   * Notifies the execution listeners for this command that an attempt to
   * execute has failed because the command is not defined.
   *
   * @param e
   *            The exception that is about to be thrown; never
   *            <code>null</code>.
   * @since 3.2
   */
  void FireNotDefined(const NotDefinedException* e);

  /**
   * Notifies the execution listeners for this command that an attempt to
   * execute has failed because there is no handler.
   *
   * @param e
   *            The exception that is about to be thrown; never
   *            <code>null</code>.
   * @since 3.2
   */
  void FireNotEnabled(const NotEnabledException* e);

  /**
   * Notifies the execution listeners for this command that an attempt to
   * execute has failed because there is no handler.
   *
   * @param e
   *            The exception that is about to be thrown; never
   *            <code>null</code>.
   */
  void FireNotHandled(const NotHandledException* e);

  /**
   * Notifies the execution listeners for this command that an attempt to
   * execute has failed during the execution.
   *
   * @param e
   *            The exception that has been thrown; never <code>null</code>.
   *            After this method completes, the exception will be thrown
   *            again.
   */
  void FirePostExecuteFailure(const ExecutionException* e);

  /**
   * Notifies the execution listeners for this command that an execution has
   * completed successfully.
   *
   * @param returnValue
   *            The return value from the command; may be <code>null</code>.
   */
  void FirePostExecuteSuccess(const Object::Pointer returnValue);

  /**
   * Notifies the execution listeners for this command that an attempt to
   * execute is about to start.
   *
   * @param event
   *            The execution event that will be used; never <code>null</code>.
   */
  void FirePreExecute(const SmartPointer<const ExecutionEvent> event);

public:

  /**
   * Returns the current handler for this command. This is used by the command
   * manager for determining the appropriate help context identifiers and by
   * the command service to allow handlers to update elements.
   * <p>
   * This value can change at any time and should never be cached.
   * </p>
   *
   * @return The current handler for this command; may be <code>null</code>.
   * @since 3.3
   */
  SmartPointer<IHandler> GetHandler() const;

  /**
   * Returns the help context identifier associated with this command. This
   * method should not be called by clients. Clients should use
   * {@link CommandManager#getHelpContextId(Command)} instead.
   *
   * @return The help context identifier for this command; may be
   *         <code>null</code> if there is none.
   * @since 3.2
   */
  std::string GetHelpContextId() const;

  /**
   * Returns the parameter with the provided id or <code>null</code> if this
   * command does not have a parameter with the id.
   *
   * @param parameterId
   *            The id of the parameter to retrieve.
   * @return The parameter with the provided id or <code>null</code> if this
   *         command does not have a parameter with the id.
   * @throws NotDefinedException
   *             If the handle is not currently defined.
   * @since 3.2
   */
  SmartPointer<IParameter> GetParameter(const std::string& parameterId) const;

  /**
   * Returns the parameters for this command. This call triggers provides a
   * copy of the array, so excessive calls to this method should be avoided.
   *
   * @return The parameters for this command. This value might be
   *         <code>null</code>, if the command has no parameters.
   * @throws NotDefinedException
   *             If the handle is not currently defined.
   */
  std::vector<SmartPointer<IParameter> > GetParameters() const;

  /**
   * Returns the {@link ParameterType} for the parameter with the provided id
   * or <code>null</code> if this command does not have a parameter type
   * with the id.
   *
   * @param parameterId
   *            The id of the parameter to retrieve the {@link ParameterType}
   *            of.
   * @return The {@link ParameterType} for the parameter with the provided id
   *         or <code>null</code> if this command does not have a parameter
   *         type with the provided id.
   * @throws NotDefinedException
   *             If the handle is not currently defined.
   * @since 3.2
   */
  SmartPointer<ParameterType> GetParameterType(const std::string& parameterId) const;

  /**
   * Returns the {@link ParameterType} for the return value of this command or
   * <code>null</code> if this command does not declare a return value
   * parameter type.
   *
   * @return The {@link ParameterType} for the return value of this command or
   *         <code>null</code> if this command does not declare a return
   *         value parameter type.
   * @throws NotDefinedException
   *             If the handle is not currently defined.
   * @since 3.2
   */
  SmartPointer<ParameterType> GetReturnType() const;

  /**
   * Returns whether this command has a handler, and whether this handler is
   * also handled and enabled.
   *
   * @return <code>true</code> if the command is handled; <code>false</code>
   *         otherwise.
   */
  bool IsEnabled() const;

  /**
   * Called be the framework to allow the handler to update its enabled state.
   *
   * @param evaluationContext
   *            the state to evaluate against. May be <code>null</code>
   *            which indicates that the handler can query whatever model that
   *            is necessary.  This context must not be cached.
   * @since 3.4
   */
  void SetEnabled(Object::ConstPointer evaluationContext);

  /**
   * Returns whether this command has a handler, and whether this handler is
   * also handled.
   *
   * @return <code>true</code> if the command is handled; <code>false</code>
   *         otherwise.
   */
  bool IsHandled() const;

  /**
   * Removes a listener from this command.
   *
   * @param commandListener
   *            The listener to be removed; must not be <code>null</code>.
   *
   */
  void RemoveCommandListener(
      const ICommandListener::Pointer commandListener);

  /**
   * Removes a listener from this command.
   *
   * @param executionListener
   *            The listener to be removed; must not be <code>null</code>.
   *
   */
  void RemoveExecutionListener(
      const IExecutionListener::Pointer executionListener);

  /**
   * <p>
   * Removes a state from this command. This will remove the state from the
   * active handler, if the active handler is an instance of
   * {@link IObjectWithState}.
   * </p>
   *
   * @param stateId
   *            The identifier of the state to remove; must not be
   *            <code>null</code>.
   * @since 3.2
   */
  void RemoveState(const std::string& stateId);

  /**
   * Changes the handler for this command. This will remove all the state from
   * the currently active handler (if any), and add it to <code>handler</code>.
   * If debugging is turned on, then this will also print information about
   * the change to <code>System.out</code>.
   *
   * @param handler
   *            The new handler; may be <code>null</code> if none.
   * @return <code>true</code> if the handler changed; <code>false</code>
   *         otherwise.
   */
  bool SetHandler(const SmartPointer<IHandler> handler);

private:

  struct HandlerListener : public IHandlerListener
  {

    HandlerListener(Command* command);

    void HandlerChanged(SmartPointer<HandlerEvent> handlerEvent);

  private:
    Command* command;
  };

  /**
   * @return the handler listener
   */
  SmartPointer<IHandlerListener> GetHandlerListener();


public:

  /**
   * The string representation of this command -- for debugging purposes only.
   * This string should not be shown to an end user.
   *
   * @return The string representation; never <code>null</code>.
   */
  std::string ToString() const;

  /**
   * Makes this command become undefined. This has the side effect of changing
   * the name and description to <code>null</code>. This also removes all
   * state and disposes of it. Notification is sent to all listeners.
   */
  void Undefine();
};

}
#endif /*CHERRYCOMMAND_H_*/
