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

#include "mitkStateMachineCondition.h"

mitk::StateMachineCondition::StateMachineCondition(const std::string &conditionName, const bool inverted)
  : m_ConditionName(conditionName), m_Inverted(inverted)
{
}

mitk::StateMachineCondition::~StateMachineCondition()
{
}

std::string mitk::StateMachineCondition::GetConditionName() const
{
  return m_ConditionName;
}

bool mitk::StateMachineCondition::IsInverted() const
{
  return m_Inverted;
}
