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

#include "cherryCommandEvent.h"
#include "cherryCommand.h"
#include "cherryCommandCategory.h"
#include "cherryIParameterValueConverter.h"
#include "cherryIHandler.h"

namespace cherry
{

const int CommandEvent::CHANGED_CATEGORY = CommandEvent::LAST_USED_BIT << 1;
const int CommandEvent::CHANGED_HANDLED = CommandEvent::LAST_USED_BIT << 2;
const int CommandEvent::CHANGED_PARAMETERS = CommandEvent::LAST_USED_BIT << 3;
const int CommandEvent::CHANGED_RETURN_TYPE = CommandEvent::LAST_USED_BIT << 4;
const int CommandEvent::CHANGED_HELP_CONTEXT_ID = CommandEvent::LAST_USED_BIT
    << 5;
const int CommandEvent::CHANGED_ENABLED = CommandEvent::LAST_USED_BIT << 6;

CommandEvent::CommandEvent(const Command::Pointer command,
    bool categoryChanged, bool definedChanged, bool descriptionChanged,
    bool handledChanged, bool nameChanged, bool parametersChanged,
    bool returnTypeChanged, bool helpContextIdChanged,
    bool enabledChanged) :
  AbstractNamedHandleEvent(definedChanged, descriptionChanged, nameChanged),
      command(command)
{

  if (!command)
  {
    throw Poco::NullPointerException("The command cannot be null");
  }

  if (categoryChanged)
  {
    changedValues |= CHANGED_CATEGORY;
  }
  if (handledChanged)
  {
    changedValues |= CHANGED_HANDLED;
  }
  if (parametersChanged)
  {
    changedValues |= CHANGED_PARAMETERS;
  }
  if (returnTypeChanged)
  {
    changedValues |= CHANGED_RETURN_TYPE;
  }
  if (helpContextIdChanged)
  {
    changedValues |= CHANGED_HELP_CONTEXT_ID;
  }
  if (enabledChanged)
  {
    changedValues |= CHANGED_ENABLED;
  }
}

Command::Pointer CommandEvent::GetCommand() const
{
  return command;
}

bool CommandEvent::IsCategoryChanged() const
{
  return ((changedValues & CHANGED_CATEGORY) != 0);
}

bool CommandEvent::IsHandledChanged() const
{
  return ((changedValues & CHANGED_HANDLED) != 0);
}

bool CommandEvent::IsHelpContextIdChanged() const
{
  return ((changedValues & CHANGED_HELP_CONTEXT_ID) != 0);
}

bool CommandEvent::IsParametersChanged() const
{
  return ((changedValues & CHANGED_PARAMETERS) != 0);
}

bool CommandEvent::IsReturnTypeChanged() const
{
  return ((changedValues & CHANGED_RETURN_TYPE) != 0);
}

bool CommandEvent::IsEnabledChanged() const
{
  return ((changedValues & CHANGED_ENABLED) != 0);
}

}
