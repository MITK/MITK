/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandEvent.h"
#include "berryCommand.h"
#include "berryCommandCategory.h"
#include "berryState.h"
#include "berryIParameterValueConverter.h"
#include "berryIHandler.h"

namespace berry
{

int CommandEvent::CHANGED_CATEGORY() { static int i = CommandEvent::LAST_USED_BIT() << 1; return i; }
int CommandEvent::CHANGED_HANDLED() { static int i = CommandEvent::LAST_USED_BIT() << 2; return i; }
int CommandEvent::CHANGED_PARAMETERS() { static int i = CommandEvent::LAST_USED_BIT() << 3; return i; }
int CommandEvent::CHANGED_RETURN_TYPE() { static int i = CommandEvent::LAST_USED_BIT() << 4; return i; }
int CommandEvent::CHANGED_HELP_CONTEXT_ID() { static int i = CommandEvent::LAST_USED_BIT() << 5; return i; }
int CommandEvent::CHANGED_ENABLED() { static int i = CommandEvent::LAST_USED_BIT() << 6; return i; }

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
    changedValues |= CHANGED_CATEGORY();
  }
  if (handledChanged)
  {
    changedValues |= CHANGED_HANDLED();
  }
  if (parametersChanged)
  {
    changedValues |= CHANGED_PARAMETERS();
  }
  if (returnTypeChanged)
  {
    changedValues |= CHANGED_RETURN_TYPE();
  }
  if (helpContextIdChanged)
  {
    changedValues |= CHANGED_HELP_CONTEXT_ID();
  }
  if (enabledChanged)
  {
    changedValues |= CHANGED_ENABLED();
  }
}

Command::Pointer CommandEvent::GetCommand() const
{
  return command;
}

bool CommandEvent::IsCategoryChanged() const
{
  return ((changedValues & CHANGED_CATEGORY()) != 0);
}

bool CommandEvent::IsHandledChanged() const
{
  return ((changedValues & CHANGED_HANDLED()) != 0);
}

bool CommandEvent::IsHelpContextIdChanged() const
{
  return ((changedValues & CHANGED_HELP_CONTEXT_ID()) != 0);
}

bool CommandEvent::IsParametersChanged() const
{
  return ((changedValues & CHANGED_PARAMETERS()) != 0);
}

bool CommandEvent::IsReturnTypeChanged() const
{
  return ((changedValues & CHANGED_RETURN_TYPE()) != 0);
}

bool CommandEvent::IsEnabledChanged() const
{
  return ((changedValues & CHANGED_ENABLED()) != 0);
}

}
