/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
