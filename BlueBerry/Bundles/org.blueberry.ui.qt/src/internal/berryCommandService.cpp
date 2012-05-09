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


#include "berryCommandService.h"

#include <berryCommand.h>
#include <berryCommandManager.h>
#include <berryCommandCategory.h>
#include <berryParameterizedCommand.h>
#include <berryUIElement.h>

#include "berryWorkbenchPlugin.h"
#include "berryElementReference.h"

#include <berryIHandler.h>
#include <berryIElementUpdater.h>
#include <berryIElementReference.h>
#include <berryISafeRunnable.h>
#include <berrySafeRunner.h>
#include <berryObjectString.h>

#include <QStringList>

namespace berry {

const QString CommandService::PREFERENCE_KEY_PREFIX = "org.blueberry.ui.commands/state";

const QString CommandService::CreatePreferenceKey(const SmartPointer<Command>& command,
                                         const QString& stateId)
{
  return PREFERENCE_KEY_PREFIX + '/' + QString::fromStdString(command->GetId()) + '/' + stateId;
}

CommandService::CommandService( CommandManager* commandManager)
  : commandManager(commandManager)
{
  if (commandManager == 0)
  {
    throw std::invalid_argument("Cannot create a command service with a null manager");
  }
  //this.commandPersistence = new CommandPersistence(this);
}

CommandService::~CommandService()
{
  this->Dispose();
}

void CommandService::AddExecutionListener(IExecutionListener* listener)
{
  commandManager->AddExecutionListener(listener);
}

void CommandService::DefineUncategorizedCategory(const QString& name,
                                 const QString& description)
{
  commandManager->DefineUncategorizedCategory(name.toStdString(), description.toStdString());
}

SmartPointer<ParameterizedCommand> CommandService::Deserialize(const QString& serializedParameterizedCommand) const
{
  return commandManager->Deserialize(serializedParameterizedCommand.toStdString());
}

void CommandService::Dispose()
{
  //commandPersistence.dispose();

  /*
   * All state on all commands neeeds to be disposed. This is so that the
   * state has a chance to persist any changes.
   */
//  final Command[] commands = commandManager.getAllCommands();
//  for (int i = 0; i < commands.length; i++) {
//    final Command command = commands[i];
//    final String[] stateIds = command.getStateIds();
//    for (int j = 0; j < stateIds.length; j++) {
//      final String stateId = stateIds[j];
//      final State state = command.getState(stateId);
//      if (state instanceof PersistentState) {
//        final PersistentState persistentState = (PersistentState) state;
//        if (persistentState.shouldPersist()) {
//          persistentState.save(PrefUtil
//              .getInternalPreferenceStore(),
//              createPreferenceKey(command, stateId));
//        }
//      }
//    }
//  }
  commandCallbacks.clear();
}

SmartPointer<CommandCategory> CommandService::GetCategory(const QString& categoryId) const
{
  return commandManager->GetCategory(categoryId.toStdString());
}

SmartPointer<Command> CommandService::GetCommand(const QString& commandId) const
{
  return commandManager->GetCommand(commandId.toStdString());
}

QList<SmartPointer<CommandCategory> > CommandService::GetDefinedCategories() const
{
  std::vector<CommandCategory::Pointer> categories = commandManager->GetDefinedCategories();
  QList<CommandCategory::Pointer> res;
  for (std::vector<CommandCategory::Pointer>::iterator i = categories.begin();
       i != categories.end(); ++i)
  {
    res.push_back(*i);
  }
  return res;
}

QStringList CommandService::GetDefinedCategoryIds() const
{
  QStringList res;
  Poco::HashSet<std::string> ids = commandManager->GetDefinedCategoryIds();
  for (Poco::HashSet<std::string>::Iterator i = ids.begin(); i != ids.end(); ++i)
  {
    res.push_back(QString::fromStdString(*i));
  }
  return res;
}

QStringList CommandService::GetDefinedCommandIds() const
{
  QStringList res;
  Poco::HashSet<std::string> ids = commandManager->GetDefinedCommandIds();
  for (Poco::HashSet<std::string>::Iterator i = ids.begin(); i != ids.end(); ++i)
  {
    res.push_back(QString::fromStdString(*i));
  }
  return res;
}

QList<SmartPointer<Command> > CommandService::GetDefinedCommands() const
{
  QList<Command::Pointer> res;
  std::vector<Command::Pointer> cmds = commandManager->GetDefinedCommands();
  for (std::vector<Command::Pointer>::iterator i = cmds.begin();
       i != cmds.end(); ++i)
  {
    res.push_back(*i);
  }
  return res;
}

QStringList CommandService::GetDefinedParameterTypeIds() const
{
  QStringList res;
  Poco::HashSet<std::string> ids = commandManager->GetDefinedParameterTypeIds();
  for (Poco::HashSet<std::string>::Iterator i = ids.begin(); i != ids.end(); ++i)
  {
    res.push_back(QString::fromStdString(*i));
  }
  return res;
}

QList<SmartPointer<ParameterType> > CommandService::GetDefinedParameterTypes() const
{
  QList<ParameterType::Pointer> res;
  std::vector<ParameterType::Pointer> cmds = commandManager->GetDefinedParameterTypes();
  for (std::vector<ParameterType::Pointer>::iterator i = cmds.begin();
       i != cmds.end(); ++i)
  {
    res.push_back(*i);
  }
  return res;
}

QString CommandService::GetHelpContextId(const SmartPointer<const Command>& command) const
{
  return QString::fromStdString(commandManager->GetHelpContextId(command));
}

QString CommandService::GetHelpContextId(const QString& commandId) const
{
  Command::Pointer command = GetCommand(commandId);
  return QString::fromStdString(commandManager->GetHelpContextId(command));
}

SmartPointer<ParameterType> CommandService::GetParameterType(const QString& parameterTypeId) const
{
  return commandManager->GetParameterType(parameterTypeId.toStdString());
}

void CommandService::ReadRegistry()
{
  //commandPersistence.read();
}

void CommandService::RemoveExecutionListener(IExecutionListener* listener)
{
  commandManager->RemoveExecutionListener(listener);
}

void CommandService::SetHelpContextId(const SmartPointer<IHandler>& handler,
                      const QString& helpContextId)
{
  commandManager->SetHelpContextId(handler, helpContextId.toStdString());
}

void CommandService::RefreshElements(const QString& commandId,
                     const QHash<QString, Object::Pointer>&  filter)
{
  Command::Pointer cmd = GetCommand(commandId);

  if (!cmd->IsDefined() || !(cmd->GetHandler().Cast<IElementUpdater>()))
  {
    return;
  }
  IElementUpdater::Pointer updater = cmd->GetHandler().Cast<IElementUpdater>();

  if (commandCallbacks.isEmpty())
  {
    return;
  }

  if(!commandCallbacks.contains(commandId))
  {
    return;
  }

  foreach (IElementReference::Pointer callbackRef, commandCallbacks[commandId])
  {
    struct _SafeRunnable : public ISafeRunnable
    {
      IElementUpdater* updater;
      IElementReference* callbackRef;

      _SafeRunnable(IElementUpdater* updater, IElementReference* callbackRef)
        : updater(updater), callbackRef(callbackRef)
      {}

      void HandleException(const std::exception& exc)
      {
        WorkbenchPlugin::Log(std::string("Failed to update callback: ") +
                             callbackRef->GetCommandId().toStdString() + exc.what());
      }

      void Run()
      {
        updater->UpdateElement(callbackRef->GetElement().GetPointer(), callbackRef->GetParameters());
      }
    };

    QHash<QString,Object::Pointer> parms = callbackRef->GetParameters();

    ISafeRunnable::Pointer run(new _SafeRunnable(updater.GetPointer(), callbackRef.GetPointer()));
    if (filter.isEmpty())
    {
      SafeRunner::Run(run);
    }
    else
    {
      bool match = true;
      QHashIterator<QString, Object::Pointer> i(filter);
      while (i.hasNext())
      {
        i.next();
        Object::Pointer value = parms[i.key()];
        if (i.value() != value)
        {
          match = false;
          break;
        }
      }
      if (match)
      {
        SafeRunner::Run(run);
      }
    }
  }
}

SmartPointer<IElementReference> CommandService::RegisterElementForCommand(
    const SmartPointer<ParameterizedCommand>& command,
    const SmartPointer<UIElement>& element)
{
  if (!command->GetCommand()->IsDefined())
  {
    throw NotDefinedException(
        "Cannot define a callback for undefined command "
            + command->GetCommand()->GetId());
  }
  if (element.IsNull())
  {
    throw NotDefinedException("No callback defined for command "
        + command->GetCommand()->GetId());
  }

  std::map<std::string, std::string> paramMap = command->GetParameterMap();
  QHash<QString, Object::Pointer> parms;
  for (std::map<std::string, std::string>::iterator i = paramMap.begin();
       i != paramMap.end(); ++i)
  {
    Object::Pointer value(new ObjectString(i->second));
    parms.insert(QString::fromStdString(i->first), value);
  }
  IElementReference::Pointer ref(new ElementReference(QString::fromStdString(command->GetId()),
                                                      element, parms));
  RegisterElement(ref);
  return ref;
}

void CommandService::RegisterElement(const SmartPointer<IElementReference>& elementReference)
{
  QList<IElementReference::Pointer>& parameterizedCommands = commandCallbacks[elementReference->GetCommandId()];
  parameterizedCommands.push_back(elementReference);

  // If the active handler wants to update the callback, it can do
  // so now
  Command::Pointer command = GetCommand(elementReference->GetCommandId());
  if (command->IsDefined())
  {
    if (IElementUpdater::Pointer updater = command->GetHandler().Cast<IElementUpdater>())
    {
      updater->UpdateElement(elementReference->GetElement().GetPointer(),
                             elementReference->GetParameters());
    }
  }
}

void CommandService::UnregisterElement(const SmartPointer<IElementReference>& elementReference)
{
  if (commandCallbacks.contains(elementReference->GetCommandId()))
  {
    QList<IElementReference::Pointer>& parameterizedCommands = commandCallbacks[elementReference->GetCommandId()];
    parameterizedCommands.removeAll(elementReference);
    if (parameterizedCommands.isEmpty())
    {
      commandCallbacks.remove(elementReference->GetCommandId());
    }
  }
}

//CommandPersistence* CommandService::GetCommandPersistence() const
//{
//  return commandPersistence;
//}

}
