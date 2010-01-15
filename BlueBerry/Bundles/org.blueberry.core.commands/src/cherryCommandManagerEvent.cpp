/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryCommandManagerEvent.h"

#include "cherryCommandManager.h"
#include "cherryIHandler.h"
#include "cherryCommandCategory.h"
#include "cherryParameterType.h"

namespace cherry
{

const int CommandManagerEvent::CHANGED_CATEGORY_DEFINED = 1;
const int CommandManagerEvent::CHANGED_COMMAND_DEFINED = 1 << 1;
const int CommandManagerEvent::CHANGED_PARAMETER_TYPE_DEFINED = 1 << 2;

CommandManagerEvent::CommandManagerEvent(CommandManager& commandManager,
    const std::string& commandId, const bool commandIdAdded,
    const bool commandIdChanged, const std::string& categoryId,
    const bool categoryIdAdded, const bool categoryIdChanged) :
  categoryId(categoryId), commandId(commandId), commandManager(commandManager)
{
  //    if (!commandManager) {
  //      throw Poco::NullPointerException(
  //          "An event must refer to its command manager"); //$NON-NLS-1$
  //    }

  if (commandIdChanged && (commandId.empty()))
  {
    throw Poco::NullPointerException(
        "If the list of defined commands changed, then the added/removed command must be mentioned"); //$NON-NLS-1$
  }

  if (categoryIdChanged && (categoryId.empty()))
  {
    throw Poco::NullPointerException(
        "If the list of defined categories changed, then the added/removed category must be mentioned"); //$NON-NLS-1$
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
    const std::string& parameterTypeId, const bool parameterTypeIdAdded,
    const bool parameterTypeIdChanged) :
  parameterTypeId(parameterTypeId), commandManager(commandManager)
{

  //    if (!commandManager) {
  //      throw Poco::NullPointerException(
  //          "An event must refer to its command manager"); //$NON-NLS-1$
  //    }

  if (parameterTypeIdChanged && (parameterTypeId.empty()))
  {
    throw Poco::NullPointerException(
        "If the list of defined command parameter types changed, then the added/removed parameter type must be mentioned"); //$NON-NLS-1$
  }

  int changedValues = 0;
  if (parameterTypeIdChanged && parameterTypeIdAdded)
  {
    changedValues |= CHANGED_PARAMETER_TYPE_DEFINED;
  }

  this->changedValues = changedValues;
}

std::string CommandManagerEvent::GetCategoryId() const
{
  return categoryId;
}

std::string CommandManagerEvent::GetCommandId() const
{
  return commandId;
}

CommandManager& CommandManagerEvent::GetCommandManager() const
{
  return commandManager;
}

std::string CommandManagerEvent::GetParameterTypeId() const
{
  return parameterTypeId;
}

bool CommandManagerEvent::IsCategoryChanged() const
{
  return (!categoryId.empty());
}

bool CommandManagerEvent::IsCategoryDefined() const
{
  return (((changedValues & CHANGED_CATEGORY_DEFINED) != 0)
      && (!categoryId.empty()));
}

bool CommandManagerEvent::IsCommandChanged() const
{
  return (!commandId.empty());
}

bool CommandManagerEvent::IsCommandDefined() const
{
  return (((changedValues & CHANGED_COMMAND_DEFINED) != 0)
      && (!commandId.empty()));
}

bool CommandManagerEvent::IsParameterTypeChanged() const
{
  return (!parameterTypeId.empty());
}

bool CommandManagerEvent::IsParameterTypeDefined() const
{
  return (((changedValues & CHANGED_PARAMETER_TYPE_DEFINED) != 0)
      && (!parameterTypeId.empty()));
}

}
