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

  void AddExecutionListener(IExecutionListener* listener);

  void DefineUncategorizedCategory(const QString& name, const QString& description);

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

  void RefreshElements(const QString& commandId, const QHash<QString, Object::Pointer>& filter);

  SmartPointer<IElementReference> RegisterElementForCommand(
      const SmartPointer<ParameterizedCommand>& command,
      const SmartPointer<UIElement>& element);

  void RegisterElement(const SmartPointer<IElementReference>& elementReference);

  void UnregisterElement(const SmartPointer<IElementReference>& elementReference);
};

}

#endif // BERRYSLAVECOMMANDSERVICE_H
