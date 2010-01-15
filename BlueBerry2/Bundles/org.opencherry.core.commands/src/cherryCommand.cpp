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

#include "cherryCommand.h"

#include "cherryIParameter.h"
#include "cherryITypedParameter.h"
#include "cherryIHandler.h"
#include "cherryIParameterValueConverter.h"
#include "cherryHandlerEvent.h"
#include "cherryCommandEvent.h"
#include "cherryExecutionEvent.h"
#include "cherryCommandCategory.h"

#include "util/cherryCommandTracing.h"
#include "internal/cherryCommandUtils.h"

#include <sstream>

namespace cherry {

bool Command::DEBUG_COMMAND_EXECUTION = false;

bool Command::DEBUG_HANDLERS = false;

std::string Command::DEBUG_HANDLERS_COMMAND_ID = "";


  Command::Command(const std::string& id) :
  NamedHandleObjectWithState(id)
  {

  }

  void Command::AddCommandListener(const ICommandListener::Pointer commandListener) {
    if (!commandListener) {
      throw Poco::NullPointerException("Cannot add a null command listener");
    }
    commandEvents.AddListener(commandListener);
  }

  void Command::AddExecutionListener(
      const IExecutionListener::Pointer executionListener) {
    if (!executionListener) {
      throw Poco::NullPointerException(
          "Cannot add a null execution listener"); //$NON-NLS-1$
    }
    executionEvents.AddListener(executionListener);
  }

  void Command::AddState(const std::string& id, const State::Pointer state) {
    NamedHandleObjectWithState::AddState(id, state);
    state->SetId(id);
    if (IObjectWithState::Pointer stateHandler = handler.Cast<IObjectWithState>()) {
      stateHandler->AddState(id, state);
    }
  }

  bool Command::operator<(const Object* object) const {
    const Command* castedObject = dynamic_cast<const Command*>(object);
    int compareTo = CommandUtils::CompareObj(category, castedObject->category);
    if (compareTo == 0) {
      compareTo = CommandUtils::Compare(defined, castedObject->defined);
      if (compareTo == 0) {
        compareTo = CommandUtils::Compare(description, castedObject->description);
        if (compareTo == 0) {
          compareTo = CommandUtils::CompareObj(handler, castedObject->handler);
          if (compareTo == 0) {
            compareTo = CommandUtils::Compare(id, castedObject->id);
            if (compareTo == 0) {
              compareTo = CommandUtils::Compare(name, castedObject->name);
              if (compareTo == 0) {
                compareTo = CommandUtils::Compare(parameters,
                    castedObject->parameters);
              }
            }
          }
        }
      }
    }
    return compareTo < 0;
  }

  void Command::Define(const std::string& name, const std::string& description,
      const CommandCategory::Pointer category, const std::vector<IParameter::Pointer>& parameters,
      ParameterType::Pointer returnType, const std::string& helpContextId) {
    if (name == "") {
      throw Poco::InvalidArgumentException(
          "The name of a command cannot be empty"); //$NON-NLS-1$
    }

    if (!category) {
      throw Poco::NullPointerException(
          "The category of a command cannot be null"); //$NON-NLS-1$
    }

    const bool definedChanged = !this->defined;
    this->defined = true;

    const bool nameChanged = this->name != name;
    this->name = name;

    const bool descriptionChanged = this->description != description;
    this->description = description;

    const bool categoryChanged = this->category != category;
    this->category = category;

    const bool parametersChanged = !CommandUtils::Equals(this->parameters,
        parameters);
    this->parameters = parameters;

    const bool returnTypeChanged = this->returnType != returnType;
    this->returnType = returnType;

    const bool helpContextIdChanged = this->helpContextId != helpContextId;
    this->helpContextId = helpContextId;

    CommandEvent::Pointer event(new CommandEvent(Command::Pointer(this), categoryChanged,
        definedChanged, descriptionChanged, false, nameChanged,
        parametersChanged, returnTypeChanged, helpContextIdChanged));
    this->FireCommandChanged(event);
  }

  Object::Pointer Command::ExecuteWithChecks(const ExecutionEvent::ConstPointer event) {
    this->FirePreExecute(event);
    const IHandler::Pointer handler(this->handler);

    if (!this->IsDefined()) {
      const NotDefinedException exception(
          "Trying to execute a command that is not defined. " + this->GetId());
      this->FireNotDefined(&exception);
      throw exception;
    }

    // Perform the execution, if there is a handler.
    if (handler && handler->IsHandled()) {
      this->SetEnabled(event->GetApplicationContext());
      if (!this->IsEnabled()) {
        const NotEnabledException exception(
            "Trying to execute the disabled command " + this->GetId());
        this->FireNotEnabled(&exception);
        throw exception;
      }

      try {
        const Object::Pointer returnValue(handler->Execute(event));
        this->FirePostExecuteSuccess(returnValue);
        return returnValue;
      } catch (const ExecutionException* e) {
        this->FirePostExecuteFailure(e);
        throw e;
      }
    }

    const NotHandledException e(
        "There is no handler to execute for command " + this->GetId()); //$NON-NLS-1$
    this->FireNotHandled(&e);
    throw e;
  }

  void Command::FireCommandChanged(const CommandEvent::ConstPointer commandEvent) {
    if (!commandEvent) {
      throw Poco::NullPointerException("Cannot fire a null event");
    }

    try
    {
      commandEvents.commandChanged(commandEvent);
    }
    catch (...)
    {
      //TODO log exceptions?
    }
  }

  void Command::FireNotDefined(const NotDefinedException* e) {
    // Debugging output
    if (DEBUG_COMMAND_EXECUTION) {
      CommandTracing::PrintTrace("COMMANDS", "execute" + CommandTracing::SEPARATOR
          + "not defined: id=" + this->GetId() + "; exception=" + e->what());
    }

    executionEvents.notDefined(this->GetId(), e);
  }

  void Command::FireNotEnabled(const NotEnabledException* e) {
    // Debugging output
    if (DEBUG_COMMAND_EXECUTION) {
      CommandTracing::PrintTrace("COMMANDS", "execute" + CommandTracing::SEPARATOR
          + "not enabled: id=" + this->GetId() + "; exception=" + e->what());
    }

    executionEvents.notEnabled(this->GetId(), e);
  }

  void Command::FireNotHandled(const NotHandledException* e) {
    // Debugging output
    if (DEBUG_COMMAND_EXECUTION) {
      CommandTracing::PrintTrace("COMMANDS", "execute" + CommandTracing::SEPARATOR
          + "not handled: id=" + this->GetId() + "; exception=" + e->what());
    }

    executionEvents.notHandled(this->GetId(), e);
  }

  void Command::FirePostExecuteFailure(const ExecutionException* e) {
    // Debugging output
    if (DEBUG_COMMAND_EXECUTION) {
      CommandTracing::PrintTrace("COMMANDS", "execute" + CommandTracing::SEPARATOR
          + "failure: id=" + this->GetId() + "; exception=" + e->what());
    }

    executionEvents.postExecuteFailure(this->GetId(), e);
  }

  void Command::FirePostExecuteSuccess(const Object::Pointer returnValue) {
    // Debugging output
    if (DEBUG_COMMAND_EXECUTION) {
      CommandTracing::PrintTrace("COMMANDS", "execute" + CommandTracing::SEPARATOR
          + "success: id=" + this->GetId() + "; returnValue=" + returnValue->ToString());
    }

    executionEvents.postExecuteSuccess(this->GetId(), returnValue);
  }

  void Command::FirePreExecute(const ExecutionEvent::ConstPointer event) {
    // Debugging output
    if (DEBUG_COMMAND_EXECUTION) {
      CommandTracing::PrintTrace("COMMANDS", "execute" + CommandTracing::SEPARATOR
          + "starting: id=" + this->GetId() + "; event=" + event->ToString());
    }

    executionEvents.preExecute(this->GetId(), event);
  }

  IHandler::Pointer Command::GetHandler() const {
    return handler;
  }

  std::string Command::GetHelpContextId() const {
    return helpContextId;
  }

  IParameter::Pointer Command::GetParameter(const std::string& parameterId) const
       {
    if (!this->IsDefined()) {
      throw NotDefinedException(
          "Cannot get a parameter from an undefined command. " + id);
    }

    for (unsigned int i = 0; i < parameters.size(); i++) {
      IParameter::Pointer parameter(parameters[i]);
      if (parameter->GetId() == parameterId) {
        return parameter;
      }
    }

    return IParameter::Pointer(0);
  }

  std::vector<IParameter::Pointer> Command::GetParameters() const {
    if (!this->IsDefined()) {
      throw NotDefinedException(
          "Cannot get the parameters from an undefined command. " + id);
    }

    return parameters;
  }

  ParameterType::Pointer Command::GetParameterType(const std::string& parameterId) const
      {
    const IParameter::Pointer parameter(this->GetParameter(parameterId));
    if (ITypedParameter::Pointer parameterWithType = parameter.Cast<ITypedParameter>()) {
      return parameterWithType->GetParameterType();
    }
    return ParameterType::Pointer(0);
  }

  ParameterType::Pointer Command::GetReturnType() const {
    if (!this->IsDefined()) {
      throw NotDefinedException(
          "Cannot get the return type of an undefined command. " + id);
    }

    return returnType;
  }

  bool Command::IsEnabled() const {
    if (!handler) {
      return false;
    }

    return handler->IsEnabled();
  }

  void Command::SetEnabled(Object::ConstPointer evaluationContext) {
    handler->SetEnabled(evaluationContext);
  }

  bool Command::IsHandled() const {
    if (!handler) {
      return false;
    }

    return handler->IsHandled();
  }

  void Command::RemoveCommandListener(
      const ICommandListener::Pointer commandListener) {
    if (!commandListener) {
      throw Poco::NullPointerException(
          "Cannot remove a null command listener");
    }

    commandEvents.RemoveListener(commandListener);
  }

  /**
   * Removes a listener from this command.
   *
   * @param executionListener
   *            The listener to be removed; must not be <code>null</code>.
   *
   */
  void Command::RemoveExecutionListener(
      const IExecutionListener::Pointer executionListener) {
    if (!executionListener) {
      throw Poco::NullPointerException(
          "Cannot remove a null execution listener"); //$NON-NLS-1$
    }

    executionEvents.RemoveListener(executionListener);
  }

  void Command::RemoveState(const std::string& stateId) {
    if (IObjectWithState::Pointer stateHandler = handler.Cast<IObjectWithState>()) {
      stateHandler->RemoveState(stateId);
    }
    NamedHandleObjectWithState::RemoveState(stateId);
  }

  bool Command::SetHandler(const IHandler::Pointer handler) {
    if (handler == this->handler) {
      return false;
    }

    // Swap the state around.
    const std::vector<std::string> stateIds(this->GetStateIds());
      for (unsigned int i = 0; i < stateIds.size(); ++i) {
        const std::string stateId = stateIds[i];
        if (IObjectWithState::Pointer stateHandler = this->handler.Cast<IObjectWithState>()) {
          stateHandler->RemoveState(stateId);
        }
        if (IObjectWithState::Pointer stateHandler = handler.Cast<IObjectWithState>()) {
          const State::Pointer stateToAdd(this->GetState(stateId));
          stateHandler->AddState(stateId, stateToAdd);
        }
      }

    bool enabled = this->IsEnabled();
    if (this->handler) {
      this->handler->RemoveHandlerListener(this->GetHandlerListener());
    }

    // Update the handler, and flush the string representation.
    this->handler = handler;
    if (this->handler) {
      this->handler->AddHandlerListener(this->GetHandlerListener());
    }
    this->str = "";

    // Debugging output
    if ((DEBUG_HANDLERS)
        && ((DEBUG_HANDLERS_COMMAND_ID.empty()) || (DEBUG_HANDLERS_COMMAND_ID == id)))
    {
      std::string buffer("Command('");
      buffer += id + "') has changed to ";
      if (!handler) {
        buffer += "no handler";
      } else {
        buffer += "\'" + handler->ToString() + "' as its handler";
      }
      CommandTracing::PrintTrace("HANDLERS", buffer);
    }

    // Send notification
    CommandEvent::Pointer cmdEvent(new CommandEvent(Command::Pointer(this), false, false, false, true,
        false, false, false, false, enabled != this->IsEnabled()));
    this->FireCommandChanged(cmdEvent);

    return true;
  }

  Command::HandlerListener::HandlerListener(cherry::Command* command) : command(command)
    {}

    void Command::HandlerListener::HandlerChanged(HandlerEvent::Pointer handlerEvent) {
      bool enabledChanged = handlerEvent->IsEnabledChanged();
      bool handledChanged = handlerEvent->IsHandledChanged();
      CommandEvent::Pointer cmdEvent(new CommandEvent(Command::Pointer(command), false,
          false, false, handledChanged, false, false, false,
          false, enabledChanged));
      command->FireCommandChanged(cmdEvent);
    }

  /**
   * @return the handler listener
   */
  SmartPointer<IHandlerListener> Command::GetHandlerListener() {
  if (!handlerListener) {
    handlerListener = new HandlerListener(this);
  }
  return handlerListener;
  }

  std::string Command::ToString() const {
    if (str.empty()) {
      std::stringstream buffer;

      buffer << "Command(" << id << ',' << name << "," << std::endl << "\t\t";
        buffer << description << "," << std::endl << "\t\t" << (category ? category->ToString() : "");
        buffer << "," << std::endl << "\t\t" << (handler ? handler->ToString() : "");
        buffer << "," << std::endl << "\t\t" << "[";
        for (unsigned int i = 0; i < parameters.size(); ++i)
        {
          buffer << parameters[i]->GetId();
        }
        buffer << "]," << (returnType ? returnType->ToString() : "");
        buffer << "," << defined << ")";

      str = buffer.str();
    }
    return str;
  }

  void Command::Undefine() {
    bool enabledChanged = this->IsEnabled();

    str = "";

    const bool definedChanged = defined;
    defined = false;

    const bool nameChanged = !name.empty();
    name = "";

    const bool descriptionChanged = !description.empty();
    description = "";

    const bool categoryChanged = category;
    category = 0;

    const bool parametersChanged = !parameters.empty();
    parameters.clear();

    const bool returnTypeChanged = returnType;
    returnType = 0;

    const std::vector<std::string> stateIds(this->GetStateIds());
      if (IObjectWithState::Pointer handlerWithState = handler.Cast<IObjectWithState>()) {
        for (unsigned int i = 0; i < stateIds.size(); i++) {
          const std::string stateId(stateIds[i]);
          handlerWithState->RemoveState(stateId);

          const State::Pointer state(this->GetState(stateId));
          this->RemoveState(stateId);
          //state.dispose();
        }
      } else {
        for (unsigned int i = 0; i < stateIds.size(); ++i) {
          const std::string stateId(stateIds[i]);
          const State::Pointer state(this->GetState(stateId));
          this->RemoveState(stateId);
          //state.dispose();
        }
      }

    CommandEvent::Pointer cmdEvent(new CommandEvent(Command::Pointer(this), categoryChanged,
        definedChanged, descriptionChanged, false, nameChanged,
        parametersChanged, returnTypeChanged, false, enabledChanged));
    this->FireCommandChanged(cmdEvent);
  }

}
