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

#ifndef mitkPropertyRelations_h
#define mitkPropertyRelations_h

#include <map>
#include <mitkIPropertyRelations.h>

namespace mitk
{
  class PropertyRelations : public IPropertyRelations
  {
  public:
    PropertyRelations();
    ~PropertyRelations() override;

    using RuleResultVectorType = IPropertyRelations::RuleResultVectorType;
    using RuleIDType = IPropertyRelations::RuleIDType;
    using RuleIDVectorType = IPropertyRelations::RuleIDVectorType;

    bool AddRule(const PropertyRelationRuleBase *rule, bool overwrite = false) override;

    RuleResultVectorType GetRulesForSource(const IPropertyProvider* source) const override;

    RuleResultVectorType GetRulesForSourceCandidate(const IPropertyProvider* sourceCandidate) const override;

    RuleResultVectorType GetRulesForDestinationCandidate(const IPropertyProvider* destCandidate) const override;

    PropertyRelationRuleBase::ConstPointer GetRule(const RuleIDType &ruleID) const override;

    RuleIDVectorType GetRuleIDs() const override;

    bool HasRuleForSource(const IPropertyProvider* source) const override;

    void RemoveAllRules() override;

    void RemoveRule(const RuleIDType &ruleID) override;

  private:
    using RuleMap = std::map<const std::string, PropertyRelationRuleBase::ConstPointer>;

    PropertyRelations(const PropertyRelations &);
    PropertyRelations &operator=(const PropertyRelations &);

    RuleMap m_RuleMap;
  };

  /**Creates an unmanaged (!) instance of PropertyRelations for testing purposes.*/
  MITKCORE_EXPORT IPropertyRelations *CreateTestInstancePropertyRelations();
}

#endif
