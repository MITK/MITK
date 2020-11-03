/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandManagerEvent.h"

#include "berryCommandManager.h"
#include "berryIHandler.h"
#include "berryCommandCategory.h"
#include "berryParameterType.h"

namespace berry
{

const int CommandManagerEvent::CHANGED_CATEGORY_DEFINED = 1;
const int CommandManagerEvent::CHANGED_COMMAND_DEFINED = 1 << 1;
const int CommandManagerEvent::CHANGED_PARAMETER_TYPE_DEFINED = 1 << 2;

CommandManagerEvent::CommandManagerEvent(CommandManager& commandManager,
    const QString& commandId, const bool commandIdAdded,
    const bool commandIdChanged, const QString& categoryId,
    const bool categoryIdAdded, const bool categoryIdChanged) :
  categoryId(categoryId), commandId(commandId), commandManager(commandManager)
{
  //    if (!commandManager) {
  //      throw Poco::NullPointerException(
  //          "An event must refer to its command manager");
  //    }

  if (commandIdChanged && (commandId.isEmpty()))
  {
    throw ctkInvalidArgumentException(
        "If the list of defined commands changed, then the added/removed command must be mentioned");
  }

  if (categoryIdChanged && (categoryId.isEmpty()))
  {
    throw ctkInvalidArgumentException(
        "If the list of defined categories changed, then the added/removed category must be mentioned");
  }

  int changedValues = 0;
  if (categoryIdChanged && categoryIdAdded)
  {
    changedValues |= CHANGED_CATEGORY_DEFINED;
  }
  if (commandIdChanged && commandIdAdded)
  {
    changedValues |= CHANGED_COMMAND_DEFINED;
  }
  this->changedValues = changedValues;
}

CommandManagerEvent::CommandManagerEvent(CommandManager& commandManager,
    const QString& parameterTypeId, const bool parameterTypeIdAdded,
    const bool parameterTypeIdChanged) :
  parameterTypeId(parameterTypeId), commandManager(commandManager)
{

  //    if (!commandManager) {
  //      throw Poco::NullPointerException(
  //          "An event must refer to its command manager"); //$NON-NLS-1$
  //    }

  if (parameterTypeIdChanged && (parameterTypeId.isEmpty()))
  {
    throw ctkInvalidArgumentException(
        "If the list of defined command parameter types changed, then the added/removed parameter type must be mentioned"); //$NON-NLS-1$
  }

  int changedValues = 0;
  if (parameterTypeIdChanged && parameterTypeIdAdded)
  {
    changedValues |= CHANGED_PARAMETER_TYPE_DEFINED;
  }

  this->changedValues = changedValues;
}

QString CommandManagerEvent::GetCategoryId() const
{
  return categoryId;
}

QString CommandManagerEvent::GetCommandId() const
{
  return commandId;
}

CommandManager& CommandManagerEvent::GetCommandManager() const
{
  return commandManager;
}

QString CommandManagerEvent::GetParameterTypeId() const
{
  return parameterTypeId;
}

bool CommandManagerEvent::IsCategoryChanged() const
{
  return (!categoryId.isEmpty());
}

bool CommandManagerEvent::IsCategoryDefined() const
{
  return (((changedValues & CHANGED_CATEGORY_DEFINED) != 0)
          && (!categoryId.isEmpty()));
}

bool CommandManagerEvent::IsCommandChanged() const
{
  return (!commandId.isEmpty());
}

bool CommandManagerEvent::IsCommandDefined() const
{
  return (((changedValues & CHANGED_COMMAND_DEFINED) != 0)
          && (!commandId.isEmpty()));
}

bool CommandManagerEvent::IsParameterTypeChanged() const
{
  return (!parameterTypeId.isEmpty());
}

bool CommandManagerEvent::IsParameterTypeDefined() const
{
  return (((changedValues & CHANGED_PARAMETER_TYPE_DEFINED) != 0)
          && (!parameterTypeId.isEmpty()));
}

}
