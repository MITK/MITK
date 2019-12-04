/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandManager.h"

#include "berryIParameter.h"
#include "berryIHandler.h"
#include "berryParameterType.h"
#include "berryParameterizedCommand.h"
#include "berryParameterization.h"
#include "berryCommand.h"
#include "berryCommandCategory.h"
#include "berryState.h"

#include "berryExecutionEvent.h"
#include "berryCommandEvent.h"
#include "berryCommandCategoryEvent.h"
#include "berryCommandManagerEvent.h"
#include "berryParameterTypeEvent.h"

namespace berry {

CommandManager::ExecutionListener::ExecutionListener(CommandManager* commandManager) : commandManager(commandManager)
{

}

void CommandManager::ExecutionListener::NotDefined(const QString& commandId, const NotDefinedException* exception)
{
  commandManager->executionEvents.notDefined(commandId, exception);
}

void CommandManager::ExecutionListener::NotEnabled(const QString& commandId, const NotEnabledException* exception)
{
  commandManager->executionEvents.notEnabled(commandId, exception);
}

void CommandManager::ExecutionListener::NotHandled(const QString& commandId, const NotHandledException* exception)
{
  commandManager->executionEvents.notHandled(commandId, exception);
}

void CommandManager::ExecutionListener::PostExecuteFailure(const QString& commandId,
                                                           const ExecutionException* exception)
{
  commandManager->executionEvents.postExecuteFailure(commandId, exception);
}

void CommandManager::ExecutionListener::PostExecuteSuccess(const QString& commandId,
                                                           const Object::Pointer& returnValue)
{
  commandManager->executionEvents.postExecuteSuccess(commandId, returnValue);
}

void CommandManager::ExecutionListener::PreExecute(const QString& commandId,
                                                   const ExecutionEvent::ConstPointer& event)
{
  commandManager->executionEvents.preExecute(commandId, event);
}

CommandManager::CommandCategoryListener::CommandCategoryListener(CommandManager* commandManager)
  : commandManager(commandManager)
{

}

void CommandManager::CommandCategoryListener::CategoryChanged(const CommandCategoryEvent::ConstPointer& categoryEvent)
{
  if (categoryEvent->IsDefinedChanged())
  {
    const CommandCategory::Pointer category(categoryEvent->GetCategory());
    const QString categoryId(category->GetId());
    const bool categoryIdAdded = category->IsDefined();
    if (categoryIdAdded)
    {
      commandManager->definedCategoryIds.insert(categoryId);
    }
    else
    {
      commandManager->definedCategoryIds.remove(categoryId);
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

void CommandManager::CommandListener::CommandChanged(const SmartPointer<const CommandEvent>& commandEvent)
{
  if (commandEvent->IsDefinedChanged())
  {
    const Command::Pointer command(commandEvent->GetCommand());
    const QString commandId = command->GetId();
    const bool commandIdAdded = command->IsDefined();
    if (commandIdAdded)
    {
      commandManager->definedHandleObjects.insert(command);
    }
    else
    {
      commandManager->definedHandleObjects.remove(command);
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
    const ParameterTypeEvent::ConstPointer& parameterTypeEvent)
{
  if (parameterTypeEvent->IsDefinedChanged())
  {
    const ParameterType::Pointer parameterType(parameterTypeEvent
                                               ->GetParameterType());
    const QString parameterTypeId = parameterType->GetId();
    const bool parameterTypeIdAdded = parameterType->IsDefined();
    if (parameterTypeIdAdded)
    {
      commandManager->definedParameterTypeIds.insert(parameterTypeId);
    }
    else
    {
      commandManager->definedParameterTypeIds.remove(parameterTypeId);
    }

    CommandManagerEvent::Pointer event(new CommandManagerEvent(*commandManager,
                                                               parameterTypeId, parameterTypeIdAdded, true));
    commandManager->FireCommandManagerChanged(event);
  }
}


const QString CommandManager::AUTOGENERATED_CATEGORY_ID = "org.blueberry.core.commands.categories.autogenerated";
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

void CommandManager::AddCommandManagerListener(ICommandManagerListener* listener)
{
  commandManagerEvents.AddListener(listener);
}

void CommandManager::AddExecutionListener(IExecutionListener *listener)
{
  if (listener == nullptr)
  {
    throw ctkInvalidArgumentException(
          "Cannot add a null execution listener");
  }

  if (executionEvents.IsEmpty())
  {
    // Add an execution listener to every command.
    executionListener.reset(new ExecutionListener(this));
    for (HandleObjectsByIdMap::Iterator itr = handleObjectsById.begin();
         itr != handleObjectsById.end(); ++itr)
    {
      Command::Pointer command = itr.value().Cast<Command>();
      if (command)
      {
        command->AddExecutionListener(executionListener.data());
      }
    }
  }

  executionEvents.AddListener(listener);
}

void CommandManager::DefineUncategorizedCategory(const QString& name,
                                                 const QString& description)
{
  CommandCategory::Pointer category(this->GetCategory(AUTOGENERATED_CATEGORY_ID));
  category->Define(name, description);
}

SmartPointer<ParameterizedCommand> CommandManager::Deserialize(
    const QString& serializedParameterizedCommand)
{

  const int lparenPosition = this->UnescapedIndexOf(
        serializedParameterizedCommand, PARAMETER_START_CHAR);

  QString commandIdEscaped;
  QString serializedParameters;
  if (lparenPosition == -1)
  {
    commandIdEscaped = serializedParameterizedCommand;
  }
  else
  {
    commandIdEscaped = serializedParameterizedCommand.left(lparenPosition);

    if (serializedParameterizedCommand
        .at(serializedParameterizedCommand.size() - 1) != PARAMETER_END_CHAR)
    {
      throw SerializationException(
            "Parentheses must be balanced in serialized ParameterizedCommand");
    }

    serializedParameters = serializedParameterizedCommand.mid(
          lparenPosition + 1, // skip PARAMETER_START_CHAR
          serializedParameterizedCommand.size() - lparenPosition - 2); // skip
    // PARAMETER_END_CHAR
  }

  const QString commandId(this->Unescape(commandIdEscaped));
  Command::Pointer command(this->GetCommand(commandId));
  const QList<IParameter::Pointer> parameters(command->GetParameters());
  const QList<Parameterization>parameterizations(this->GetParameterizations(
                                                         serializedParameters, parameters));

  ParameterizedCommand::Pointer pCmd(new ParameterizedCommand(command, parameterizations));
  return pCmd;
}

QList<SmartPointer<Command> > CommandManager::GetAllCommands() const
{
  QList<Command::Pointer> result;
  for (HandleObjectsByIdMap::ConstIterator itr = handleObjectsById.begin();
       itr != handleObjectsById.end(); ++itr)
  {
    if (Command::Pointer cmd = itr.value().Cast<Command>())
    {
      result.push_back(cmd);
    }
  }
  return result;
}

SmartPointer<CommandCategory> CommandManager::GetCategory(const QString& categoryId)
{
  if (categoryId.isEmpty())
  {
    return this->GetCategory(AUTOGENERATED_CATEGORY_ID);
  }

  this->CheckId(categoryId);

  CommandCategory::Pointer category(categoriesById[categoryId]);
  if (!category)
  {
    category = new CommandCategory(categoryId);
    categoriesById[categoryId] = category;
    category->AddCategoryListener(categoryListener.data());
  }

  return category;
}

SmartPointer<Command> CommandManager::GetCommand(const QString& commandId)
{
  this->CheckId(commandId);

  Command::Pointer command(handleObjectsById[commandId].Cast<Command>());
  if (!command)
  {
    command = new Command(commandId);
    handleObjectsById[commandId] = command;
    command->AddCommandListener(commandListener.data());

    if (executionListener)
    {
      command->AddExecutionListener(executionListener.data());
    }
  }

  return command;
}

QList<CommandCategory::Pointer> CommandManager::GetDefinedCategories()
{
  QList<CommandCategory::Pointer> categories;
  for (QSet<QString>::Iterator itr = definedCategoryIds.begin();
       itr != definedCategoryIds.end(); ++itr)
  {
    categories.push_back(this->GetCategory(*itr));
  }
  return categories;
}

QSet<QString> CommandManager::GetDefinedCategoryIds() const
{
  return definedCategoryIds;
}

QSet<QString> CommandManager::GetDefinedCommandIds() const
{
  return this->GetDefinedHandleObjectIds();
}

QList<SmartPointer<Command> > CommandManager::GetDefinedCommands() const
{
  QList<Command::Pointer> result;
  for (HandleObjectsSet::ConstIterator itr = definedHandleObjects.begin();
       itr != definedHandleObjects.end(); ++itr)
  {
    if (Command::Pointer cmd = itr->Cast<Command>())
      result.push_back(cmd);
  }
  return result;
}

QSet<QString> CommandManager::GetDefinedParameterTypeIds() const
{
  return definedParameterTypeIds;
}

QList<SmartPointer<ParameterType> > CommandManager::GetDefinedParameterTypes()
{
  QList<ParameterType::Pointer> parameterTypes;
  for (QSet<QString>::ConstIterator itr = definedParameterTypeIds.begin();
       itr != definedParameterTypeIds.end(); ++itr)
  {
    parameterTypes.push_back(this->GetParameterType(*itr));
  }
  return parameterTypes;
}

QString CommandManager::GetHelpContextId(const SmartPointer<const Command> command) const
{
  // Check if the command is defined.
  if (!command->IsDefined())
  {
    throw NotDefinedException("The command is not defined. "
                              + command->GetId());
  }

  // Check the handler.
  const IHandler::Pointer handler(command->GetHandler());
  if (handler)
  {
    const IHandler::WeakPtr weakHandler(handler);
    QHash<WeakPointer<IHandler>, QString>::ConstIterator itr =
        helpContextIdsByHandler.find(weakHandler);
    if (itr != helpContextIdsByHandler.end() && !itr.value().isEmpty())
    {
      return itr.value();
    }
  }

  // Simply return whatever the command has as a help context identifier.
  return command->GetHelpContextId();
}

SmartPointer<ParameterType> CommandManager::GetParameterType(const QString& parameterTypeId)
{
  this->CheckId(parameterTypeId);

  ParameterType::Pointer parameterType(parameterTypesById[parameterTypeId]);
  if (!parameterType)
  {
    parameterType = new ParameterType(parameterTypeId);
    parameterTypesById[parameterTypeId] = parameterType;
    parameterType->AddListener(parameterTypeListener.data());
  }

  return parameterType;
}

void CommandManager::RemoveCommandManagerListener(ICommandManagerListener *listener)
{
  commandManagerEvents.RemoveListener(listener);
}

void CommandManager::RemoveExecutionListener(IExecutionListener *listener)
{
  if (!listener)
  {
    throw ctkInvalidArgumentException("Cannot remove a null listener");
  }

  if (executionEvents.IsEmpty())
  {
    return;
  }

  executionEvents.RemoveListener(listener);

  if (executionEvents.IsEmpty())
  {
    // Remove the execution listener from every command.
    for (HandleObjectsByIdMap::Iterator commandItr = handleObjectsById.begin();
         commandItr != handleObjectsById.end(); ++commandItr)
    {
      Command::Pointer command(commandItr.value().Cast<Command>());
      command->RemoveExecutionListener(executionListener.data());
    }
    executionListener.reset();
  }
}

void CommandManager::SetHandlersByCommandId(const QHash<QString, SmartPointer<IHandler> > &handlersByCommandId)
{
  // Make that all the reference commands are created.
  for (QHash<QString, SmartPointer<IHandler> >::const_iterator commandItr = handlersByCommandId.begin();
       commandItr != handlersByCommandId.end(); ++commandItr)
  {
    this->GetCommand(commandItr.key());
  }

  // Now, set-up the handlers on all of the existing commands.
  for (HandleObjectsByIdMap::Iterator commandItr = handleObjectsById.begin();
       commandItr != handleObjectsById.end(); ++commandItr)
  {
    const Command::Pointer command(commandItr.value().Cast<Command>());
    const QString commandId(command->GetId());
    QHash<QString, SmartPointer<IHandler> >::const_iterator handlerItr = handlersByCommandId.find(commandId);
    if (handlerItr != handlersByCommandId.end())
    {
      command->SetHandler(handlerItr.value());
    }
    else
    {
      command->SetHandler(IHandler::Pointer(nullptr));
    }
  }
}

void CommandManager::SetHelpContextId(const SmartPointer<IHandler> handler,
                                      const QString& helpContextId)
{
  if (!handler)
  {
    throw ctkInvalidArgumentException("The handler cannot be null");
  }
  IHandler::WeakPtr weakHandler(handler);
  if (helpContextId.isEmpty())
  {
    helpContextIdsByHandler.remove(weakHandler);
  }
  else
  {
    helpContextIdsByHandler[weakHandler] = helpContextId;
  }
}

void CommandManager::FireNotEnabled(const QString& commandId, const NotEnabledException* exception)
{
  executionEvents.notEnabled(commandId, exception);
}

void CommandManager::FireNotDefined(const QString& commandId, const NotDefinedException* exception)
{
  executionEvents.notDefined(commandId, exception);
}

void CommandManager::FirePreExecute(const QString& commandId, const SmartPointer<const ExecutionEvent> event)
{
  executionEvents.preExecute(commandId, event);
}

void CommandManager::FirePostExecuteSuccess(const QString& commandId, Object::Pointer returnValue)
{
  executionEvents.postExecuteSuccess(commandId, returnValue);
}

void CommandManager::FirePostExecuteFailure(const QString& commandId,
                                            const ExecutionException* exception)
{
  executionEvents.postExecuteFailure(commandId, exception);
}

QString CommandManager::Unescape(const QString& escapedText)
{

  // defer initialization of a StringBuffer until we know we need one
  QString buffer;

  int pos = 0;
  for (QString::const_iterator itr = escapedText.begin();
       itr != escapedText.end(); ++itr, ++pos)
  {
    QString::value_type c = *itr;
    if (c != ESCAPE_CHAR)
    {
      // normal unescaped character
      if (!buffer.isEmpty())
      {
        buffer += c;
      }
    }
    else
    {
      if (buffer.isEmpty())
      {
        buffer = escapedText.left(pos);
      }

      if (++itr != escapedText.end())
      {
        c = *itr;
        if (c == PARAMETER_START_CHAR || c == PARAMETER_END_CHAR ||
            c == ID_VALUE_CHAR || c == PARAMETER_SEPARATOR_CHAR ||
            c == ESCAPE_CHAR)
        {
          buffer += c;
        }
        else
        {
          QString msg("Invalid character '");
          msg += c;
          msg += "' in escape sequence";
          throw SerializationException(msg);
        }
      }
      else
      {
        throw SerializationException(
              "Unexpected termination of escape sequence"); //$NON-NLS-1$
      }
    }

  }

  if (buffer.isEmpty())
  {
    return escapedText;
  }

  return buffer;
}

void CommandManager::FireCommandManagerChanged(const SmartPointer<const CommandManagerEvent> event)
{
  if (!event)
  {
    throw ctkInvalidArgumentException("Event cannot be null.");
  }

  commandManagerEvents.commandManagerChanged(event);
}

QList<Parameterization> CommandManager::GetParameterizations(
    const QString& serializedParams, const QList<SmartPointer<IParameter> >& parameters) const
{

  QString serializedParameters(serializedParams);

  if (serializedParameters.isEmpty() || parameters.isEmpty())
  {
    return QList<Parameterization>();
  }

  QList<Parameterization> paramList;

  int commaPosition; // split off each param by looking for ','
  do
  {
    commaPosition = this->UnescapedIndexOf(serializedParameters, ',');

    QString idEqualsValue;
    if (commaPosition == -1)
    {
      // no more parameters after this
      idEqualsValue = serializedParameters;
    }
    else
    {
      // take the first parameter...
      idEqualsValue = serializedParameters.left(commaPosition);

      // ... and put the rest back into serializedParameters
      serializedParameters = serializedParameters.mid(commaPosition + 1);
    }

    int equalsPosition = this->UnescapedIndexOf(idEqualsValue, '=');

    QString parameterId;
    QString parameterValue;
    if (equalsPosition == -1)
    {
      // missing values are null
      parameterId = this->Unescape(idEqualsValue);
    }
    else
    {
      parameterId = this->Unescape(idEqualsValue.left(equalsPosition));
      parameterValue = this->Unescape(idEqualsValue.mid(equalsPosition + 1));
    }

    for (int i = 0; i < parameters.size(); i++)
    {
      const IParameter::Pointer parameter(parameters[i]);
      if (parameter->GetId() == parameterId)
      {
        paramList.push_back(Parameterization(parameter,
                                             parameterValue));
        break;
      }
    }

  } while (commaPosition != -1);

  return paramList;
}

int CommandManager::UnescapedIndexOf(const QString& escapedText, const char ch) const
{

  int pos = escapedText.indexOf(ch);

  // first char can't be escaped
  if (pos == 0)
  {
    return pos;
  }

  while (pos != -1)
  {
    // look back for the escape character
    if (escapedText.at(pos - 1) != ESCAPE_CHAR)
    {
      return pos;
    }

    // scan for the next instance of ch
    pos = escapedText.indexOf(ch, pos + 1);
  }

  return pos;
}

}
