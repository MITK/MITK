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

namespace cherry {

bool Command::DEBUG_COMMAND_EXECUTION = false;

bool Command::DEBUG_HANDLERS = false;

std::string Command::DEBUG_HANDLERS_COMMAND_ID = null;

  
  Command::Command(const std::string& id) {
    super(id);
  }

  void Command::AddState(const std::string& id, const State::ConstPointer state) {
    super.addState(id, state);
    state.setId(id);
    if (handler instanceof IObjectWithState) {
      ((IObjectWithState) handler).addState(id, state);
    }
  }

  int Command::CompareTo(final Object object) {
    final Command castedObject = (Command) object;
    int compareTo = Util.compare(category, castedObject.category);
    if (compareTo == 0) {
      compareTo = Util.compare(defined, castedObject.defined);
      if (compareTo == 0) {
        compareTo = Util.compare(description, castedObject.description);
        if (compareTo == 0) {
          compareTo = Util.compare(handler, castedObject.handler);
          if (compareTo == 0) {
            compareTo = Util.compare(id, castedObject.id);
            if (compareTo == 0) {
              compareTo = Util.compare(name, castedObject.name);
              if (compareTo == 0) {
                compareTo = Util.compare(parameters,
                    castedObject.parameters);
              }
            }
          }
        }
      }
    }
    return compareTo;
  }

  void Command::Define(const std::string& name, const std::string& description
      /*, final Category category*/) {
    define(name, description, category, null);
  }

  void Command::Define(const std::string& name, const std::string& description,
      /*final Category category,*/ const std::vector<IParameter::Pointer>& parameters,
      ParameterType::Pointer returnType = 0, const std::string& helpContextId = "") {
    if (name == null) {
      throw new NullPointerException(
          "The name of a command cannot be null"); //$NON-NLS-1$
    }

    if (category == null) {
      throw new NullPointerException(
          "The category of a command cannot be null"); //$NON-NLS-1$
    }

    final boolean definedChanged = !this.defined;
    this.defined = true;

    final boolean nameChanged = !Util.equals(this.name, name);
    this.name = name;

    final boolean descriptionChanged = !Util.equals(this.description,
        description);
    this.description = description;

    final boolean categoryChanged = !Util.equals(this.category, category);
    this.category = category;

    final boolean parametersChanged = !Util.equals(this.parameters,
        parameters);
    this.parameters = parameters;

    final boolean returnTypeChanged = !Util.equals(this.returnType,
        returnType);
    this.returnType = returnType;

    final boolean helpContextIdChanged = !Util.equals(this.helpContextId,
        helpContextId);
    this.helpContextId = helpContextId;

    fireCommandChanged(new CommandEvent(this, categoryChanged,
        definedChanged, descriptionChanged, false, nameChanged,
        parametersChanged, returnTypeChanged, helpContextIdChanged));
  }

  Object::Pointer Command::ExecuteWithChecks(const ExecutionEvent::ConstPointer event) {
    firePreExecute(event);
    final IHandler handler = this.handler;

    if (!isDefined()) {
      final NotDefinedException exception = new NotDefinedException(
          "Trying to execute a command that is not defined. " //$NON-NLS-1$
              + getId());
      fireNotDefined(exception);
      throw exception;
    }

    // Perform the execution, if there is a handler.
    if ((handler != null) && (handler.isHandled())) {
      setEnabled(event.getApplicationContext());
      if (!isEnabled()) {
        final NotEnabledException exception = new NotEnabledException(
            "Trying to execute the disabled command " + getId()); //$NON-NLS-1$
        fireNotEnabled(exception);
        throw exception;
      }

      try {
        final Object returnValue = handler.execute(event);
        firePostExecuteSuccess(returnValue);
        return returnValue;
      } catch (final ExecutionException e) {
        firePostExecuteFailure(e);
        throw e;
      }
    }

    final NotHandledException e = new NotHandledException(
        "There is no handler to execute for command " + getId()); //$NON-NLS-1$
    fireNotHandled(e);
    throw e;
  }

  IHandler::Pointer Command::GetHandler() {
    return handler;
  }

  std::string Command::GetHelpContextId() {
    return helpContextId;
  }

  IParameter GetParameter(const std::string& parameterId)
      throws NotDefinedException {
    if (!isDefined()) {
      throw new NotDefinedException(
          "Cannot get a parameter from an undefined command. " //$NON-NLS-1$
              + id);
    }

    if (parameters == null) {
      return null;
    }

    for (int i = 0; i < parameters.length; i++) {
      final IParameter parameter = parameters[i];
      if (parameter.getId().equals(parameterId)) {
        return parameter;
      }
    }

    return null;
  }

  std::vector<IParameter::Pointer> Command::GetParameters() {
    if (!isDefined()) {
      throw new NotDefinedException(
          "Cannot get the parameters from an undefined command. " //$NON-NLS-1$
              + id);
    }

    if ((parameters == null) || (parameters.length == 0)) {
      return null;
    }

    final IParameter[] returnValue = new IParameter[parameters.length];
    System.arraycopy(parameters, 0, returnValue, 0, parameters.length);
    return returnValue;
  }

  ParameterType GetParameterType(const std::string& parameterId)
      throws NotDefinedException {
    final IParameter parameter = getParameter(parameterId);
    if (parameter instanceof ITypedParameter) {
      final ITypedParameter parameterWithType = (ITypedParameter) parameter;
      return parameterWithType.getParameterType();
    }
    return null;
  }

  ParameterType Command::GetReturnType() {
    if (!isDefined()) {
      throw new NotDefinedException(
          "Cannot get the return type of an undefined command. " //$NON-NLS-1$
              + id);
    }

    return returnType;
  }

  bool Command::IsEnabled() {
    if (handler == null) {
      return false;
    }

    return handler.isEnabled();
  }
  
  void Command::SetEnabled(Object::Pointer evaluationContext) {
    if (handler instanceof IHandler2) {
      ((IHandler2) handler).setEnabled(evaluationContext);
    }
  }

  bool Command::IsHandled() {
    if (handler == null) {
      return false;
    }

    return handler.isHandled();
  }

  void Command::RemoveState(const std::string& stateId) {
    if (handler instanceof IObjectWithState) {
      ((IObjectWithState) handler).removeState(stateId);
    }
    super.removeState(stateId);
  }

  bool Command::SetHandler(const IHandler::ConstPointer handler) {
    if (Util.equals(handler, this.handler)) {
      return false;
    }

    // Swap the state around.
    final String[] stateIds = getStateIds();
    if (stateIds != null) {
      for (int i = 0; i < stateIds.length; i++) {
        final String stateId = stateIds[i];
        if (this.handler instanceof IObjectWithState) {
          ((IObjectWithState) this.handler).removeState(stateId);
        }
        if (handler instanceof IObjectWithState) {
          final State stateToAdd = getState(stateId);
          ((IObjectWithState) handler).addState(stateId, stateToAdd);
        }
      }
    }

    boolean enabled = isEnabled();
    if (this.handler != null) {
      this.handler.removeHandlerListener(getHandlerListener());
    }

    // Update the handler, and flush the string representation.
    this.handler = handler;
    if (this.handler != null) {
      this.handler.addHandlerListener(getHandlerListener());
    }
    string = null;

    // Debugging output
    if ((DEBUG_HANDLERS)
        && ((DEBUG_HANDLERS_COMMAND_ID == null) || (DEBUG_HANDLERS_COMMAND_ID
            .equals(id)))) {
      final StringBuffer buffer = new StringBuffer("Command('"); //$NON-NLS-1$
      buffer.append(id);
      buffer.append("') has changed to "); //$NON-NLS-1$
      if (handler == null) {
        buffer.append("no handler"); //$NON-NLS-1$
      } else {
        buffer.append('\'');
        buffer.append(handler);
        buffer.append("' as its handler"); //$NON-NLS-1$
      }
      Tracing.printTrace("HANDLERS", buffer.toString()); //$NON-NLS-1$
    }

    // Send notification
    fireCommandChanged(new CommandEvent(this, false, false, false, true,
        false, false, false, false, enabled != isEnabled()));

    return true;
  }

  void Command::PrintSelf(std::ostream& os, Indent Indent) const {
    if (string == null) {
      final StringWriter sw = new StringWriter();
      final BufferedWriter buffer = new BufferedWriter(sw);
      try {
        buffer.write("Command("); //$NON-NLS-1$
        buffer.write(id);
        buffer.write(',');
        buffer.write(name==null?"":name); //$NON-NLS-1$
        buffer.write(',');
        buffer.newLine();
        buffer.write("\t\t"); //$NON-NLS-1$
        buffer.write(description==null?"":description); //$NON-NLS-1$
        buffer.write(',');
        buffer.newLine();
        buffer.write("\t\t"); //$NON-NLS-1$
        buffer.write(category==null?"":category.toString()); //$NON-NLS-1$
        buffer.write(',');
        buffer.newLine();
        buffer.write("\t\t"); //$NON-NLS-1$
        buffer.write(handler==null?"":handler.toString()); //$NON-NLS-1$
        buffer.write(',');
        buffer.newLine();
        buffer.write("\t\t"); //$NON-NLS-1$
        buffer.write(parameters==null?"":parameters.toString()); //$NON-NLS-1$
        buffer.write(',');
        buffer.write(returnType==null?"":returnType.toString()); //$NON-NLS-1$
        buffer.write(',');
        buffer.write(""+defined); //$NON-NLS-1$
        buffer.write(')');
        buffer.flush();
      } catch (IOException e) {
        // should never get this exception
      }
      string = sw.toString();
    }
    return string;
  }

  void Command::Undefine() {
    boolean enabledChanged = isEnabled();

    string = null;

    final boolean definedChanged = defined;
    defined = false;

    final boolean nameChanged = name != null;
    name = null;

    final boolean descriptionChanged = description != null;
    description = null;

    final boolean categoryChanged = category != null;
    category = null;

    final boolean parametersChanged = parameters != null;
    parameters = null;

    final boolean returnTypeChanged = returnType != null;
    returnType = null;

    final String[] stateIds = getStateIds();
    if (stateIds != null) {
      if (handler instanceof IObjectWithState) {
        final IObjectWithState handlerWithState = (IObjectWithState) handler;
        for (int i = 0; i < stateIds.length; i++) {
          final String stateId = stateIds[i];
          handlerWithState.removeState(stateId);

          final State state = getState(stateId);
          removeState(stateId);
          state.dispose();
        }
      } else {
        for (int i = 0; i < stateIds.length; i++) {
          final String stateId = stateIds[i];
          final State state = getState(stateId);
          removeState(stateId);
          state.dispose();
        }
      }
    }

    fireCommandChanged(new CommandEvent(this, categoryChanged,
        definedChanged, descriptionChanged, false, nameChanged,
        parametersChanged, returnTypeChanged, false, enabledChanged));
  }

}
