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


#ifndef BERRYCOMMANDSERVICE_H
#define BERRYCOMMANDSERVICE_H

#include <berryICommandService.h>

#include "berryCommandPersistence.h"

namespace berry {

class CommandManager;

/**
 * <p>
 * Provides services related to the command architecture within the workbench.
 * This service can be used to access the set of commands and handlers.
 * </p>
 */
class CommandService : public ICommandService
{

private:

  friend class CommandPersistence;

  /**
   * The preference key prefix for all handler state.
   */
  static const QString PREFERENCE_KEY_PREFIX;

  /**
   * Creates a preference key for the given piece of state on the given
   * command.
   *
   * @param command
   *            The command for which the preference key should be created;
   *            must not be <code>null</code>.
   * @param stateId
   *            The identifier of the state for which the preference key
   *            should be created; must not be <code>null</code>.
   * @return A suitable preference key; never <code>null</code>.
   */
  static const QString CreatePreferenceKey(const SmartPointer<Command>& command,
                                           const QString& stateId);

  /**
   * The command manager that supports this service. This value is never
   * <code>null</code>.
   */
  CommandManager* commandManager;

  /**
   * The persistence class for this command service.
   */
  CommandPersistence commandPersistence;

  /**
   * This is a map of commandIds to a list containing currently registered
   * callbacks, in the form of IElementReference.
   */
  QHash<QString, QList<SmartPointer<IElementReference> > > commandCallbacks;


public:

  /**
   * Constructs a new instance of <code>CommandService</code> using a
   * command manager. Takes ownership of the provided CommandManager.
   *
   * @param commandManager
   *            The command manager to use; must not be <code>null</code>.
   */
  CommandService( CommandManager* commandManager);

  ~CommandService();

  void AddExecutionListener(IExecutionListener* listener);

  void DefineUncategorizedCategory(const QString& name,
                                   const QString& description);

  SmartPointer<ParameterizedCommand> Deserialize(const QString& serializedParameterizedCommand) const;

  void Dispose();

  SmartPointer<CommandCategory> GetCategory(const QString& categoryId) const;

  SmartPointer<Command> GetCommand(const QString& commandId) const;

  QList<SmartPointer<CommandCategory> > GetDefinedCategories() const;

  QStringList GetDefinedCategoryIds() const;

  QStringList GetDefinedCommandIds() const;

  QList<SmartPointer<Command> > GetDefinedCommands() const;

  QStringList GetDefinedParameterTypeIds() const;

  QList<SmartPointer<ParameterType> > GetDefinedParameterTypes() const;

  QString GetHelpContextId(const SmartPointer<const Command>& command) const;

  QString GetHelpContextId(const QString& commandId) const;

  SmartPointer<ParameterType> GetParameterType(const QString& parameterTypeId) const;

  void ReadRegistry();

  void RemoveExecutionListener(IExecutionListener* listener);

  void SetHelpContextId(const SmartPointer<IHandler>& handler,
                        const QString& helpContextId);

  void RefreshElements(const QString& commandId,
                       const QHash<QString, Object::Pointer>&  filter);

  SmartPointer<IElementReference> RegisterElementForCommand(
      const SmartPointer<ParameterizedCommand>& command,
      const SmartPointer<UIElement>& element);

  void RegisterElement(const SmartPointer<IElementReference>& elementReference);

  void UnregisterElement(const SmartPointer<IElementReference>& elementReference);

  const CommandPersistence* GetCommandPersistence() const;
};

}

#endif // BERRYCOMMANDSERVICE_H
