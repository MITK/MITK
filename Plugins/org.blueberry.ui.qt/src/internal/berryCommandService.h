/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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

  ~CommandService() override;

  void AddExecutionListener(IExecutionListener* listener) override;

  void DefineUncategorizedCategory(const QString& name,
                                   const QString& description) override;

  SmartPointer<ParameterizedCommand> Deserialize(const QString& serializedParameterizedCommand) const override;

  void Dispose() override;

  SmartPointer<CommandCategory> GetCategory(const QString& categoryId) const override;

  SmartPointer<Command> GetCommand(const QString& commandId) const override;

  QList<SmartPointer<CommandCategory> > GetDefinedCategories() const override;

  QStringList GetDefinedCategoryIds() const override;

  QStringList GetDefinedCommandIds() const override;

  QList<SmartPointer<Command> > GetDefinedCommands() const override;

  QStringList GetDefinedParameterTypeIds() const override;

  QList<SmartPointer<ParameterType> > GetDefinedParameterTypes() const override;

  QString GetHelpContextId(const SmartPointer<const Command>& command) const override;

  QString GetHelpContextId(const QString& commandId) const override;

  SmartPointer<ParameterType> GetParameterType(const QString& parameterTypeId) const override;

  void ReadRegistry() override;

  void RemoveExecutionListener(IExecutionListener* listener) override;

  void SetHelpContextId(const SmartPointer<IHandler>& handler,
                        const QString& helpContextId) override;

  void RefreshElements(const QString& commandId,
                       const QHash<QString, Object::Pointer>&  filter) override;

  SmartPointer<IElementReference> RegisterElementForCommand(
      const SmartPointer<ParameterizedCommand>& command,
      const SmartPointer<UIElement>& element) override;

  void RegisterElement(const SmartPointer<IElementReference>& elementReference) override;

  void UnregisterElement(const SmartPointer<IElementReference>& elementReference) override;

  const CommandPersistence* GetCommandPersistence() const;
};

}

#endif // BERRYCOMMANDSERVICE_H
