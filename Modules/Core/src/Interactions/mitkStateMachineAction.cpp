/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkStateMachineAction.h"

mitk::StateMachineAction::StateMachineAction(const std::string &actionName) : m_ActionName(actionName)
{
}

std::string mitk::StateMachineAction::GetActionName() const
{
  return m_ActionName;
}

mitk::StateMachineAction::~StateMachineAction()
{
}
