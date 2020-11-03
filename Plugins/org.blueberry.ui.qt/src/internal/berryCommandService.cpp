/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryCommandService.h"

#include <berryCommand.h>
#include <berryCommandManager.h>
#include <berryCommandCategory.h>
#include <berryParameterizedCommand.h>
#include <berryUIElement.h>

#include "berryPersistentState.h"
#include "berryWorkbenchPlugin.h"
#include "berryElementReference.h"
#include "berryIPreferences.h"

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
  return PREFERENCE_KEY_PREFIX + '/' + command->GetId() + '/' + stateId;
}

CommandService::CommandService( CommandManager* commandManager)
  : commandManager(commandManager)
  , commandPersistence(this)
{
  if (commandManager == nullptr)
  {
    throw std::invalid_argument("Cannot create a command service with a null manager");
  }
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
  commandManager->DefineUncategorizedCategory(name, description);
}

SmartPointer<ParameterizedCommand> CommandService::Deserialize(const QString& serializedParameterizedCommand) const
{
  return commandManager->Deserialize(serializedParameterizedCommand);
}

void CommandService::Dispose()
{
  /*
   * All state on all commands neeeds to be disposed. This is so that the
   * state has a chance to persist any changes.
   */
  const QList<Command::Pointer> commands = commandManager->GetAllCommands();
  foreach (const Command::Pointer& command, commands)
  {
    const QList<QString> stateIds = command->GetStateIds();
    foreach(const QString& stateId, stateIds)
    {
      const State::Pointer state = command->GetState(stateId);
      if (PersistentState::Pointer persistentState = state.Cast<PersistentState>())
      {
        if (persistentState->ShouldPersist())
        {
          persistentState->Save(WorkbenchPlugin::GetDefault()->GetPreferences(),
                                CreatePreferenceKey(command, stateId));
        }
      }
    }
  }
  commandCallbacks.clear();
}

SmartPointer<CommandCategory> CommandService::GetCategory(const QString& categoryId) const
{
  return commandManager->GetCategory(categoryId);
}

SmartPointer<Command> CommandService::GetCommand(const QString& commandId) const
{
  return commandManager->GetCommand(commandId);
}

QList<SmartPointer<CommandCategory> > CommandService::GetDefinedCategories() const
{
  return commandManager->GetDefinedCategories();
}

QStringList CommandService::GetDefinedCategoryIds() const
{
  return commandManager->GetDefinedCategoryIds().toList();
}

QStringList CommandService::GetDefinedCommandIds() const
{
  return commandManager->GetDefinedCommandIds().toList();
}

QList<SmartPointer<Command> > CommandService::GetDefinedCommands() const
{
  return commandManager->GetDefinedCommands();
}

QStringList CommandService::GetDefinedParameterTypeIds() const
{
  return commandManager->GetDefinedParameterTypeIds().toList();
}

QList<SmartPointer<ParameterType> > CommandService::GetDefinedParameterTypes() const
{
  return commandManager->GetDefinedParameterTypes();
}

QString CommandService::GetHelpContextId(const SmartPointer<const Command>& command) const
{
  return commandManager->GetHelpContextId(command);
}

QString CommandService::GetHelpContextId(const QString& commandId) const
{
  Command::Pointer command = GetCommand(commandId);
  return commandManager->GetHelpContextId(command);
}

SmartPointer<ParameterType> CommandService::GetParameterType(const QString& parameterTypeId) const
{
  return commandManager->GetParameterType(parameterTypeId);
}

void CommandService::ReadRegistry()
{
  commandPersistence.Read();
}

void CommandService::RemoveExecutionListener(IExecutionListener* listener)
{
  commandManager->RemoveExecutionListener(listener);
}

void CommandService::SetHelpContextId(const SmartPointer<IHandler>& handler,
                      const QString& helpContextId)
{
  commandManager->SetHelpContextId(handler, helpContextId);
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

      void HandleException(const ctkException& exc) override
      {
        WorkbenchPlugin::Log(QString("Failed to update callback: ") +
                             callbackRef->GetCommandId() + exc.what());
      }

      void Run() override
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

  QHash<QString, QString> paramMap = command->GetParameterMap();
  QHash<QString, Object::Pointer> parms;
  for (QHash<QString, QString>::const_iterator i = paramMap.begin();
       i != paramMap.end(); ++i)
  {
    Object::Pointer value(new ObjectString(i.value()));
    parms.insert(i.key(), value);
  }
  IElementReference::Pointer ref(new ElementReference(command->GetId(),
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

const CommandPersistence* CommandService::GetCommandPersistence() const
{
  return &commandPersistence;
}

}
