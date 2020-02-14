/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSLAVECOMMANDSERVICE_H
#define BERRYSLAVECOMMANDSERVICE_H

#include "berryICommandService.h"

namespace berry {

/**
 * A command service which delegates almost all responsibility to the parent
 * service.
 * <p>
 * This class is not intended for use outside of the
 * <code>org.blueberry.ui</code> plug-in.
 * </p>
 */
class SlaveCommandService : public ICommandService
{

private:

  QList<IExecutionListener*> fExecutionListeners;

  /**
   * The collection of ICallbackReferences added through this service.
   */
  QSet<IElementReference*> fCallbackCache;

  ICommandService* const fParentService;

  /**
   * The scoping constant added to callback registrations submitted through
   * this service.
   */
  const QString fScopingName;

  /**
   * The object to scope. In theory, the service locator that would find this
   * service.
   */
  const Object::Pointer fScopingValue;

public:

  /**
   * Build the slave service.
   *
   * @param parent
   *            the parent service. This must not be <code>null</code>.
   */
  SlaveCommandService(ICommandService* parent, const QString& scopeName,
                      const Object::Pointer& scopeValue);

  void AddExecutionListener(IExecutionListener* listener) override;

  void DefineUncategorizedCategory(const QString& name, const QString& description) override;

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

  void RefreshElements(const QString& commandId, const QHash<QString, Object::Pointer>& filter) override;

  SmartPointer<IElementReference> RegisterElementForCommand(
      const SmartPointer<ParameterizedCommand>& command,
      const SmartPointer<UIElement>& element) override;

  void RegisterElement(const SmartPointer<IElementReference>& elementReference) override;

  void UnregisterElement(const SmartPointer<IElementReference>& elementReference) override;
};

}

#endif // BERRYSLAVECOMMANDSERVICE_H
