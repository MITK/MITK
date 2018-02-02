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

#include <algorithm>
#include <regex>
#include <utility>

#include <mitkPropertyRelations.h>

mitk::PropertyRelations::PropertyRelations()
{
}

mitk::PropertyRelations::~PropertyRelations()
{
}

bool mitk::PropertyRelations::AddRule(const PropertyRelationRuleBase *rule, bool overwrite)
{
  if (!rule)
  {
    return false;
  }

  if (rule->GetRuleID().empty())
  {
    return false;
  }

  auto ruleFinding = m_RuleMap.find(rule->GetRuleID());

  bool result = false;
  if (ruleFinding == m_RuleMap.end())
  {
    m_RuleMap.insert(std::make_pair(rule->GetRuleID(), rule));
    result = true;
  }
  else if (overwrite)
  {
    m_RuleMap[rule->GetRuleID()] = rule;
    result = true;
  }

  return result;
}

mitk::PropertyRelations::RuleResultVectorType mitk::PropertyRelations::GetRulesForSource(const IPropertyProvider* source) const
{
  RuleResultVectorType result;

  if (source)
  {
    for (const auto rule : m_RuleMap)
    {
      if (rule.second->IsSource(source))
      {
        result.push_back(rule.second);
      }
    }
  }

  return result;
};

mitk::PropertyRelations::RuleResultVectorType mitk::PropertyRelations::GetRulesForSourceCandidate(const IPropertyProvider* sourceCandidate) const
{
  RuleResultVectorType result;

  for (const auto rule : m_RuleMap)
  {
    if (rule.second->IsSourceCandidate(sourceCandidate))
    {
      result.push_back(rule.second);
    }
  }

  return result;
};

mitk::PropertyRelations::RuleResultVectorType mitk::PropertyRelations::GetRulesForDestinationCandidate(const IPropertyProvider* destCandidate) const
{
  RuleResultVectorType result;

  for (const auto rule : m_RuleMap)
  {
    if (rule.second->IsDestinationCandidate(destCandidate))
    {
      result.push_back(rule.second);
    }
  }

  return result;
};

mitk::PropertyRelationRuleBase::ConstPointer mitk::PropertyRelations::GetRule(const RuleIDType &ruleID) const
{
  mitk::PropertyRelationRuleBase::ConstPointer result;

  auto ruleFinding = m_RuleMap.find(ruleID);

  if (ruleFinding != m_RuleMap.end())
  {
    result = ruleFinding->second;
  }

  return result;
};

mitk::PropertyRelations::RuleIDVectorType mitk::PropertyRelations::GetRuleIDs() const
{
  RuleIDVectorType result;

  for (const auto& rule : m_RuleMap)
  {
    result.push_back(rule.first);
  }
  return result;
};


bool mitk::PropertyRelations::HasRuleForSource(const IPropertyProvider* source) const
{
  return !this->GetRulesForSource(source).empty();
};

void mitk::PropertyRelations::RemoveAllRules()
{
  m_RuleMap.clear();
}

void mitk::PropertyRelations::RemoveRule(const RuleIDType &ruleID)
{
  if (!ruleID.empty())
  {
    m_RuleMap.erase(ruleID);
  }
}


mitk::IPropertyRelations *mitk::CreateTestInstancePropertyRelations()
{
  return new PropertyRelations();
};