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

#ifndef mitkIPropertyRelations_h
#define mitkIPropertyRelations_h

#include <mitkPropertyRelationRuleBase.h>
#include <mitkServiceInterface.h>

namespace mitk
{
  /** \ingroup MicroServices_Interfaces
    * \brief Interface of property relations service
    *
    * This service allows you to manage relations between property provider based on properties.
    * Property relations are managed by rule classes derived from PropertyRelationRuleBase.
    *
    */
  class MITKCORE_EXPORT IPropertyRelations
  {
  public:
    virtual ~IPropertyRelations();

    using RuleResultVectorType = std::list<PropertyRelationRuleBase::ConstPointer>;
    using RuleIDType = PropertyRelationRuleBase::RuleIDType;
    using RuleIDVectorType = std::vector<RuleIDType>;

    /** \brief Add rule for a specific relation.
    * If there is already a property rule instance  it won't be added.
    * Rule instances are regarded equal, if the rule ID is equal.
    * You may enforce to overwrite the old equal rule
    * by the overwrite parameter.
    *
    * \param[in] rule Relation rule of the property.
    * \param[in] overwrite Overwrite already existing relation rule.
    * \return True if relation rule was added successfully.
    */
    virtual bool AddRule(const PropertyRelationRuleBase *rule, bool overwrite = false) = 0;

    /** \brief Get all relation rules the passed source is really a source of. (PropertyRelationRuleBase::IsSource() would return true)
    * \param[in] source Pointer to the instance that should be checked as source for the searched rules.
    * \return Property relation rules or empty list if no relation rules are available.
    */
    virtual RuleResultVectorType GetRulesForSource(const IPropertyProvider* source) const = 0;

    /** \brief Get all relation rules that would accept the passed IPropertOwner as source.
    * \param[in] sourceCandidate Pointer to the instance that should be checked to be a suitable source.
    * \return Property relation rules or empty list if no relation rules are available.
    */
    virtual RuleResultVectorType GetRulesForSourceCandidate(const IPropertyProvider* sourceCandidate) const = 0;

    /** \brief Get all relation rules that would accept the passed IPropertOwner as destination.
    * \param[in] destCandidate Pointer to the instance that should be checked to be a suitable destination.
    * \return Property relation rules or empty list if no relation rules are available.
    */
    virtual RuleResultVectorType GetRulesForDestinationCandidate(const IPropertyProvider* destCandidate) const = 0;

    /** \brief Get the relation rule that has the specfied ruleID.
    *
    * \return Property relation rule or null pointer, if no relation rule is available.
    */
    virtual PropertyRelationRuleBase::ConstPointer GetRule(const RuleIDType &ruleID) const = 0;

    /** \brief Get all IDs of registered rules */
    virtual RuleIDVectorType GetRuleIDs() const = 0;

    /** \brief Check if a passed instance has defined relation covered by the registered rules.
    *
    * \param[in] source Pointer to the instance that should be checked as source for the searched rules.
    * \return True if the property has relations of registered rules, false otherwise.
    */
    virtual bool HasRuleForSource(const IPropertyProvider* source) const = 0;

    /** \brief Remove all relation rules.
    */
    virtual void RemoveAllRules() = 0;

    /** \brief Remove relation rule instance with the passed ID. If rule does not exist nothing happens.
    */
    virtual void RemoveRule(const RuleIDType &ruleID) = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyRelations, "org.mitk.IPropertyRelations")

#endif
