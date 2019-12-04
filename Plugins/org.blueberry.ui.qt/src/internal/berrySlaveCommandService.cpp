/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySlaveCommandService.h"

#include "berryCommandExceptions.h"
#include "berryElementReference.h"
#include "berryCommand.h"
#include "berryParameterizedCommand.h"
#include "berryCommandCategory.h"

#include "berryObjectString.h"

#include <QStringList>

namespace berry {

SlaveCommandService::SlaveCommandService(ICommandService* parent, const QString& scopeName,
                    const Object::Pointer& scopeValue)
  : fParentService(parent)
  , fScopingName(scopeName)
  , fScopingValue(scopeValue)
{
  if (parent == nullptr)
  {
    throw ctkInvalidArgumentException("The parent command service must not be null");
  }
}

void SlaveCommandService::AddExecutionListener(IExecutionListener* listener)
{
  if (!fExecutionListeners.contains(listener))
  {
    fExecutionListeners.push_back(listener);
  }
  fParentService->AddExecutionListener(listener);
}

void SlaveCommandService::DefineUncategorizedCategory(const QString& name, const QString& description)
{
  fParentService->DefineUncategorizedCategory(name, description);
}

SmartPointer<ParameterizedCommand> SlaveCommandService::Deserialize(const QString& serializedParameterizedCommand) const
{
  return fParentService->Deserialize(serializedParameterizedCommand);
}

void SlaveCommandService::Dispose()
{
  if (!fExecutionListeners.isEmpty())
  {
    for (int i = 0; i < fExecutionListeners.size(); i++)
    {
      RemoveExecutionListener(fExecutionListeners[i]);
    }
    fExecutionListeners.clear();
  }
  foreach(IElementReference* ref, fCallbackCache)
  {
    UnregisterElement(IElementReference::Pointer(ref));
  }
  fCallbackCache.clear();
}

SmartPointer<CommandCategory> SlaveCommandService::GetCategory(const QString& categoryId) const
{
  return fParentService->GetCategory(categoryId);
}

SmartPointer<Command> SlaveCommandService::GetCommand(const QString& commandId) const
{
  return fParentService->GetCommand(commandId);
}

QList<SmartPointer<CommandCategory> > SlaveCommandService::GetDefinedCategories() const
{
  return fParentService->GetDefinedCategories();
}

QStringList SlaveCommandService::GetDefinedCategoryIds() const
{
  return fParentService->GetDefinedCategoryIds();
}

QStringList SlaveCommandService::GetDefinedCommandIds() const
{
  return fParentService->GetDefinedCommandIds();
}

QList<SmartPointer<Command> > SlaveCommandService::GetDefinedCommands() const
{
  return fParentService->GetDefinedCommands();
}

QStringList SlaveCommandService::GetDefinedParameterTypeIds() const
{
  return fParentService->GetDefinedParameterTypeIds();
}

QList<SmartPointer<ParameterType> > SlaveCommandService::GetDefinedParameterTypes() const
{
  return fParentService->GetDefinedParameterTypes();
}

QString SlaveCommandService::GetHelpContextId(const SmartPointer<const Command>& command) const
{
  return fParentService->GetHelpContextId(command);
}

QString SlaveCommandService::GetHelpContextId(const QString& commandId) const
{
  return fParentService->GetHelpContextId(commandId);
}

SmartPointer<ParameterType> SlaveCommandService::GetParameterType(const QString& parameterTypeId) const
{
  return fParentService->GetParameterType(parameterTypeId);
}

void SlaveCommandService::ReadRegistry()
{
  fParentService->ReadRegistry();
}

void SlaveCommandService::RemoveExecutionListener(IExecutionListener* listener)
{
  fExecutionListeners.removeAll(listener);
  fParentService->RemoveExecutionListener(listener);
}

void SlaveCommandService::SetHelpContextId(const SmartPointer<IHandler>& handler,
                      const QString& helpContextId)
{
  fParentService->SetHelpContextId(handler, helpContextId);
}

void SlaveCommandService::RefreshElements(const QString& commandId, const QHash<QString, Object::Pointer>& filter)
{
  fParentService->RefreshElements(commandId, filter);
}

SmartPointer<IElementReference> SlaveCommandService::RegisterElementForCommand(
    const SmartPointer<ParameterizedCommand>& command,
    const SmartPointer<UIElement>& element)
{
  if (!command->GetCommand()->IsDefined())
  {
    throw NotDefinedException(QString("Cannot define a callback for undefined command ")
                              + command->GetCommand()->GetId());
  }
  if (element.IsNull())
  {
    throw NotDefinedException(QString("No callback defined for command ")
                              + command->GetCommand()->GetId());
  }

  QHash<QString, Object::Pointer> elemParams;
  QHash<QString, QString> paramMap = command->GetParameterMap();
  QHashIterator<QString,QString> iter(paramMap);
  while(iter.hasNext())
  {
    iter.next();
    elemParams.insert(iter.key(), Object::Pointer(new ObjectString(iter.value())));
  }

  ElementReference::Pointer ref(
        new ElementReference(command->GetId(), element, elemParams));
  RegisterElement(ref);
  return ref;
}

void SlaveCommandService::RegisterElement(const SmartPointer<IElementReference>& elementReference)
{
  fCallbackCache.insert(elementReference.GetPointer());
  elementReference.Cast<ElementReference>()->AddParameter(fScopingName, fScopingValue);
  fParentService->RegisterElement(elementReference);
}

void SlaveCommandService::UnregisterElement(const SmartPointer<IElementReference>& elementReference)
{
  fCallbackCache.remove(elementReference.GetPointer());
  fParentService->UnregisterElement(elementReference);
}

}
