/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyRelations_h
#define mitkPropertyRelations_h

#include <map>
#include <mitkIPropertyRelations.h>

namespace mitk
{
  /** \brief Concrete implementation of the IPropertyRelations service interface.
   *
   * Manages property relation rules in an internal map keyed by rule ID.
   * Rules define how property providers (typically data nodes) can be related
   * to each other via properties.
   *
   * \sa IPropertyRelations
   * \sa PropertyRelationRuleBase
   */
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

  /** \brief Create an unmanaged instance of PropertyRelations for testing purposes.
   *
   * \warning The caller is responsible for the lifetime of the returned pointer.
   * This function is intended for unit tests only.
   */
  MITKCORE_EXPORT IPropertyRelations *CreateTestInstancePropertyRelations();
}

#endif
