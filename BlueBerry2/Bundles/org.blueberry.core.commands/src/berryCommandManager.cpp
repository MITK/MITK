/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryCommandManager.h"

#include "berryIParameter.h"
#include "berryIHandler.h"
#include "berryParameterType.h"
#include "berryParameterizedCommand.h"
#include "berryParameterization.h"
#include "berryCommand.h"
#include "berryCommandCategory.h"

#include "berryExecutionEvent.h"
#include "berryCommandEvent.h"
#include "berryCommandCategoryEvent.h"
#include "berryCommandManagerEvent.h"
#include "berryParameterTypeEvent.h"

namespace berry {

 CommandManager::ExecutionListener::ExecutionListener(CommandManager* commandManager) : commandManager(commandManager)
    {

    }

    void CommandManager::ExecutionListener::NotDefined(const std::string& commandId, const NotDefinedException* exception) {
      commandManager->executionEvents.notDefined(commandId, exception);
    }

    void CommandManager::ExecutionListener::NotEnabled(const std::string& commandId, const NotEnabledException* exception) {
      commandManager->executionEvents.notEnabled(commandId, exception);
    }

    void CommandManager::ExecutionListener::NotHandled(const std::string& commandId, const NotHandledException* exception) {
      commandManager->executionEvents.notHandled(commandId, exception);
    }

    void CommandManager::ExecutionListener::PostExecuteFailure(const std::string& commandId,
        const ExecutionException* exception) {
      commandManager->executionEvents.postExecuteFailure(commandId, exception);
    }

    void CommandManager::ExecutionListener::PostExecuteSuccess(const std::string& commandId,
        const Object::Pointer returnValue) {
      commandManager->executionEvents.postExecuteSuccess(commandId, returnValue);
    }

    void CommandManager::ExecutionListener::PreExecute(const std::string& commandId,
        const ExecutionEvent::ConstPointer event) {
      commandManager->executionEvents.preExecute(commandId, event);
    }

    CommandManager::CommandCategoryListener::CommandCategoryListener(CommandManager* commandManager)
    : commandManager(commandManager)
    {

    }

  void CommandManager::CommandCategoryListener::CategoryChanged(const CommandCategoryEvent::ConstPointer categoryEvent) {
    if (categoryEvent->IsDefinedChanged()) {
      const CommandCategory::Pointer category(categoryEvent->GetCategory());
      const std::string categoryId(category->GetId());
      const bool categoryIdAdded = category->IsDefined();
      if (categoryIdAdded) {
        commandManager->definedCategoryIds.insert(categoryId);
      } else {
        commandManager->definedCategoryIds.erase(categoryId);
      }
       CommandManagerEvent::Pointer event(new CommandManagerEvent(*commandManager, "",
            false, false, categoryId, categoryIdAdded, true));
        commandManager->FireCommandManagerChanged(event);

    }
  }

    CommandManager::CommandListener::CommandListener(CommandManager* commandManager)
    : commandManager(commandManager)
    {

    }

  void CommandManager::CommandListener::CommandChanged(const SmartPointer<const CommandEvent> commandEvent) {
    if (commandEvent->IsDefinedChanged()) {
      const Command::Pointer command(commandEvent->GetCommand());
      const std::string commandId = command->GetId();
      const bool commandIdAdded = command->IsDefined();
      if (commandIdAdded) {
        commandManager->definedHandleObjects.insert(command);
      } else {
        commandManager->definedHandleObjects.erase(command);
      }
      CommandManagerEvent::Pointer event(new CommandManagerEvent(*commandManager,
            commandId, commandIdAdded, true, "", false, false));
        commandManager->FireCommandManagerChanged(event);

    }
  }

    CommandManager::ParameterTypeListener::ParameterTypeListener(CommandManager* commandManager)
    : commandManager(commandManager)
    {

    }

   void CommandManager::ParameterTypeListener::ParameterTypeChanged(
      const ParameterTypeEvent::ConstPointer parameterTypeEvent) {
    if (parameterTypeEvent->IsDefinedChanged()) {
      const ParameterType::Pointer parameterType(parameterTypeEvent
          ->GetParameterType());
      const std::string parameterTypeId = parameterType->GetId();
      const bool parameterTypeIdAdded = parameterType->IsDefined();
      if (parameterTypeIdAdded) {
        commandManager->definedParameterTypeIds.insert(parameterTypeId);
      } else {
        commandManager->definedParameterTypeIds.erase(parameterTypeId);
      }

      CommandManagerEvent::Pointer event(new CommandManagerEvent(*commandManager,
          parameterTypeId, parameterTypeIdAdded, true));
      commandManager->FireCommandManagerChanged(event);
    }
  }


  const std::string CommandManager::AUTOGENERATED_CATEGORY_ID = "org.blueberry.core.commands.categories.autogenerated";
  const char CommandManager::ESCAPE_CHAR = '%';
const char CommandManager::ID_VALUE_CHAR = '=';
const char CommandManager::PARAMETER_END_CHAR = ')';
 const char CommandManager::PARAMETER_SEPARATOR_CHAR = ',';
const char CommandManager::PARAMETER_START_CHAR = '(';

CommandManager::CommandManager()
 : categoryListener(new CommandCategoryListener(this)),
   commandListener(new CommandListener(this)),
   parameterTypeListener(new ParameterTypeListener(this))
 {

 }

  void CommandManager::AddCommandManagerListener(
      const SmartPointer<ICommandManagerListener> listener) {
    commandManagerEvents.AddListener(listener);
  }

  void CommandManager::AddExecutionListener(const SmartPointer<IExecutionListener> listener) {
    if (!listener) {
      throw Poco::NullPointerException(
          "Cannot add a null execution listener"); //$NON-NLS-1$
    }

    if (executionEvents.IsEmpty()) {

      // Add an execution listener to every command.
      executionListener = new ExecutionListener(this);
      for (HandleObjectsByIdMap::Iterator itr = handleObjectsById.begin();
           itr != handleObjectsById.end(); ++itr)
      {
        Command::Pointer command = itr->second.Cast<Command>();
        if (command)
        {
          command->AddExecutionListener(executionListener);
        }
      }
    }

    executionEvents.AddListener(listener);
  }

  void CommandManager::DefineUncategorizedCategory(const std::string& name,
      const std::string& description) {
    CommandCategory::Pointer category(this->GetCategory(AUTOGENERATED_CATEGORY_ID));
    category->Define(name, description);
  }

  SmartPointer<ParameterizedCommand> CommandManager::Deserialize(
      const std::string& serializedParameterizedCommand)
      throw(NotDefinedException, SerializationException) {

    const int lparenPosition = this->UnescapedIndexOf(
        serializedParameterizedCommand, PARAMETER_START_CHAR);

    std::string commandIdEscaped;
    std::string serializedParameters;
    if (lparenPosition == -1) {
      commandIdEscaped = serializedParameterizedCommand;
    } else {
      commandIdEscaped = serializedParameterizedCommand.substr(0,
          lparenPosition);

      if (serializedParameterizedCommand
          .at(serializedParameterizedCommand.size() - 1) != PARAMETER_END_CHAR) {
        throw SerializationException(
            "Parentheses must be balanced in serialized ParameterizedCommand"); //$NON-NLS-1$
      }

      serializedParameters = serializedParameterizedCommand.substr(
          lparenPosition + 1, // skip PARAMETER_START_CHAR
          serializedParameterizedCommand.size() - 1); // skip
      // PARAMETER_END_CHAR
    }

    const std::string commandId(this->Unescape(commandIdEscaped));
    Command::Pointer command(this->GetCommand(commandId));
    const std::vector<IParameter::Pointer> parameters(command->GetParameters());
    const std::vector<Parameterization>parameterizations(this->GetParameterizations(
        serializedParameters, parameters));

    ParameterizedCommand::Pointer pCmd(new ParameterizedCommand(command, parameterizations));
    return pCmd;
  }

  std::vector<SmartPointer<Command> > CommandManager::GetAllCommands() const {
    std::vector<Command::Pointer> result;
    for (HandleObjectsByIdMap::ConstIterator itr = handleObjectsById.begin();
         itr != handleObjectsById.end(); ++itr)
    {
      if (Command::Pointer cmd = itr->second.Cast<Command>())
      {
        result.push_back(cmd);
      }
    }
    return result;
  }

  SmartPointer<CommandCategory> CommandManager::GetCategory(const std::string& categoryId) {
    if (categoryId.empty()) {
      return this->GetCategory(AUTOGENERATED_CATEGORY_ID);
    }

    this->CheckId(categoryId);

    CommandCategory::Pointer category(categoriesById[categoryId]);
    if (!category) {
      category = new CommandCategory(categoryId);
      categoriesById[categoryId] = category;
      category->AddCategoryListener(categoryListener);
    }

    return category;
  }

  SmartPointer<Command> CommandManager::GetCommand(const std::string& commandId) {
    this->CheckId(commandId);

    Command::Pointer command(handleObjectsById[commandId].Cast<Command>());
    if (!command) {
      command = new Command(commandId);
      handleObjectsById[commandId] = command;
      command->AddCommandListener(commandListener);

      if (executionListener) {
        command->AddExecutionListener(executionListener);
      }
    }

    return command;
  }

  std::vector<CommandCategory::Pointer> CommandManager::GetDefinedCategories() {
    std::vector<CommandCategory::Pointer> categories;
    for (Poco::HashSet<std::string>::Iterator itr = definedCategoryIds.begin();
         itr != definedCategoryIds.end(); ++itr)
    {
      categories.push_back(this->GetCategory(*itr));
    }
    return categories;
  }

  Poco::HashSet<std::string> CommandManager::GetDefinedCategoryIds() const {
    return definedCategoryIds;
  }

  Poco::HashSet<std::string> CommandManager::GetDefinedCommandIds() const {
    return this->GetDefinedHandleObjectIds();
  }

  std::vector<SmartPointer<Command> > CommandManager::GetDefinedCommands() const {
    std::vector<Command::Pointer> result;
    for (HandleObjectsSet::ConstIterator itr = definedHandleObjects.begin();
         itr != definedHandleObjects.end(); ++itr)
    {
      if (Command::Pointer cmd = itr->Cast<Command>())
        result.push_back(cmd);
    }
    return result;
  }

  Poco::HashSet<std::string> CommandManager::GetDefinedParameterTypeIds() const {
    return definedParameterTypeIds;
  }

  std::vector<SmartPointer<ParameterType> > CommandManager::GetDefinedParameterTypes() {
    std::vector<ParameterType::Pointer> parameterTypes;
    for (Poco::HashSet<std::string>::ConstIterator itr = definedParameterTypeIds.begin();
         itr != definedParameterTypeIds.end(); ++itr)
    {
      parameterTypes.push_back(this->GetParameterType(*itr));
    }
    return parameterTypes;
  }

  std::string CommandManager::GetHelpContextId(const SmartPointer<const Command> command) const
      throw(NotDefinedException) {
    // Check if the command is defined.
    if (!command->IsDefined()) {
      throw NotDefinedException("The command is not defined. " //$NON-NLS-1$
          + command->GetId());
    }

    // Check the handler.
    const IHandler::Pointer handler(command->GetHandler());
    if (handler) {
      const IHandler::WeakPtr weakHandler(handler);
      Poco::HashMap<WeakPointer<IHandler>, std::string, Object::Hash>::ConstIterator itr =
        helpContextIdsByHandler.find(weakHandler);
      if (itr != helpContextIdsByHandler.end() && !itr->second.empty())
      {
        return itr->second;
      }
    }

    // Simply return whatever the command has as a help context identifier.
    return command->GetHelpContextId();
  }

  SmartPointer<ParameterType> CommandManager::GetParameterType(const std::string& parameterTypeId) {
    this->CheckId(parameterTypeId);

    ParameterType::Pointer parameterType(parameterTypesById[parameterTypeId]);
    if (!parameterType) {
      parameterType = new ParameterType(parameterTypeId);
      parameterTypesById[parameterTypeId] = parameterType;
      parameterType->AddListener(parameterTypeListener);
    }

    return parameterType;
  }

  void CommandManager::RemoveCommandManagerListener(
      const SmartPointer<ICommandManagerListener> listener) {
    commandManagerEvents.RemoveListener(listener);
  }

  void CommandManager::RemoveExecutionListener(const SmartPointer<IExecutionListener> listener) {
    if (!listener) {
      throw Poco::NullPointerException("Cannot remove a null listener");
    }

    if (executionEvents.IsEmpty()) {
      return;
    }

    executionEvents.RemoveListener(listener);

    if (executionEvents.IsEmpty()) {
      // Remove the execution listener to every command.
      for (HandleObjectsByIdMap::Iterator commandItr = handleObjectsById.begin();
           commandItr != handleObjectsById.end(); ++commandItr)
      {
        Command::Pointer command(commandItr->second.Cast<Command>());
        command->RemoveExecutionListener(executionListener);
      }
      executionListener = 0;

    }
  }

  void CommandManager::SetHandlersByCommandId(const std::map<std::string, SmartPointer<IHandler> >& handlersByCommandId) {
    // Make that all the reference commands are created.
    for (std::map<std::string, SmartPointer<IHandler> >::const_iterator commandItr = handlersByCommandId.begin();
         commandItr != handlersByCommandId.end(); ++commandItr)
    {
      this->GetCommand(commandItr->first);
    }

    // Now, set-up the handlers on all of the existing commands.
    for (HandleObjectsByIdMap::Iterator commandItr = handleObjectsById.begin();
         commandItr != handleObjectsById.end(); ++commandItr)
    {
      const Command::Pointer command(commandItr->second.Cast<Command>());
      const std::string commandId(command->GetId());
      std::map<std::string, SmartPointer<IHandler> >::const_iterator handlerItr = handlersByCommandId.find(commandId);
      if (handlerItr != handlersByCommandId.end())
      {
        command->SetHandler(handlerItr->second);
      }
      else
      {
        command->SetHandler(IHandler::Pointer(0));
      }
    }
  }

  void CommandManager::SetHelpContextId(const SmartPointer<IHandler> handler,
      const std::string& helpContextId) {
    if (!handler) {
      throw Poco::NullPointerException("The handler cannot be null");
    }
    IHandler::WeakPtr weakHandler(handler);
    if (helpContextId.empty()) {
      helpContextIdsByHandler.erase(weakHandler);
    } else {
      helpContextIdsByHandler[weakHandler] = helpContextId;
    }
  }

  void CommandManager::FireNotEnabled(const std::string& commandId, const NotEnabledException* exception) {
    executionEvents.notEnabled(commandId, exception);
  }

  void CommandManager::FireNotDefined(const std::string& commandId, const NotDefinedException* exception) {
      executionEvents.notDefined(commandId, exception);
  }

  void CommandManager::FirePreExecute(const std::string& commandId, const SmartPointer<const ExecutionEvent> event) {
    executionEvents.preExecute(commandId, event);
  }

  void CommandManager::FirePostExecuteSuccess(const std::string& commandId, Object::Pointer returnValue) {
      executionEvents.postExecuteSuccess(commandId, returnValue);
  }

  void CommandManager::FirePostExecuteFailure(const std::string& commandId,
      const ExecutionException* exception) {
      executionEvents.postExecuteFailure(commandId, exception);
    }

  std::string CommandManager::Unescape(const std::string& escapedText)
      throw(SerializationException) {

    // defer initialization of a StringBuffer until we know we need one
    std::string buffer;

    for (std::string::const_iterator itr = escapedText.begin();
         itr != escapedText.end(); ++itr) {

      std::string::value_type c = *itr;
      if (c != ESCAPE_CHAR) {
        // normal unescaped character
        if (!buffer.empty()) {
          buffer += c;
        }
      } else {
        if (buffer.empty()) {
          buffer.assign(escapedText.begin(), itr);
        }

        if (++itr != escapedText.end()) {
          c = *itr;
          if (c == PARAMETER_START_CHAR || c == PARAMETER_END_CHAR ||
              c == ID_VALUE_CHAR || c == PARAMETER_SEPARATOR_CHAR ||
              c == ESCAPE_CHAR)
          {
            buffer += c;
          }
          else
          {
            std::string msg("Invalid character '");
            msg += c;
            msg += "' in escape sequence";
            throw SerializationException(msg);
          }
        } else {
          throw SerializationException(
              "Unexpected termination of escape sequence"); //$NON-NLS-1$
        }
      }

    }

    if (buffer.empty()) {
      return escapedText;
    }

    return buffer;
  }

  void CommandManager::FireCommandManagerChanged(const SmartPointer<const CommandManagerEvent> event) {
    if (!event) {
      throw Poco::NullPointerException();
    }

    commandManagerEvents.commandManagerChanged(event);
  }

  std::vector<Parameterization> CommandManager::GetParameterizations(
      const std::string& serializedParams, const std::vector<SmartPointer<IParameter> >& parameters) const
      throw(SerializationException) {

    std::string serializedParameters(serializedParams);

    if (serializedParameters.empty() || parameters.empty()) {
      return std::vector<Parameterization>();
    }

    std::vector<Parameterization> paramList;

    std::string::size_type commaPosition; // split off each param by looking for ','
    do {
      commaPosition = this->UnescapedIndexOf(serializedParameters, ',');

      std::string idEqualsValue;
      if (commaPosition == std::string::npos) {
        // no more parameters after this
        idEqualsValue = serializedParameters;
      } else {
        // take the first parameter...
        idEqualsValue = serializedParameters.substr(0, commaPosition);

        // ... and put the rest back into serializedParameters
        serializedParameters = serializedParameters.substr(commaPosition + 1);
      }

      const std::string::size_type equalsPosition = this->UnescapedIndexOf(idEqualsValue, '=');

      std::string parameterId;
      std::string parameterValue;
      if (equalsPosition == std::string::npos) {
        // missing values are null
        parameterId = this->Unescape(idEqualsValue);
      } else {
        parameterId = this->Unescape(idEqualsValue.substr(0, equalsPosition));
        parameterValue = this->Unescape(idEqualsValue.substr(equalsPosition + 1));
      }

      for (unsigned int i = 0; i < parameters.size(); i++) {
        const IParameter::Pointer parameter(parameters[i]);
        if (parameter->GetId() == parameterId) {
          paramList.push_back(Parameterization(parameter,
              parameterValue));
          break;
        }
      }

    } while (commaPosition != std::string::npos);

    return paramList;
  }

  std::string::size_type CommandManager::UnescapedIndexOf(const std::string& escapedText, const char ch) const {

    std::string::size_type pos = escapedText.find_first_of(ch);

    // first char can't be escaped
    if (pos == 0) {
      return pos;
    }

    while (pos != std::string::npos) {
      // look back for the escape character
      if (escapedText.at(pos - 1) != ESCAPE_CHAR) {
        return pos;
      }

      // scan for the next instance of ch
      pos = escapedText.find_first_of(ch, pos + 1);
    }

    return pos;
  }

}
