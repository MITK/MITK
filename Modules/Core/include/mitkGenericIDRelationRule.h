/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGenericIDRelationRule_h
#define mitkGenericIDRelationRule_h

#include "mitkPropertyRelationRuleBase.h"

namespace mitk
{
  /**This rule class can be used for relations that are only defined on the ID-layer and
  where no connection on the Data-layer can be defined or deduced.
  So it can be used for all ID based relations between PropertyProviders that also implement the
  interface identifiable.
  In order to be able to use this class for different relation types based on ID, the ruleIDTag is
  used. It must be specified when creating a rule instance. The ruleIDTag will be used as suffix
  for the rule ID of the instance and therefore allows to create specific and distinguishable rules
  instances based on this class.
  One may also specify the display name and the role names of the instance. If not specified, the default values
  are used (display name: "<ruleIDTag> relation", source role name: "source of <ruleIDTag> relation",
  destination role name: "destination <ruleIDTag> of relation")
  */
  class MITKCORE_EXPORT GenericIDRelationRule : public mitk::PropertyRelationRuleBase
  {
  public:
    mitkClassMacro(GenericIDRelationRule, PropertyRelationRuleBase);
    itkCloneMacro(Self);
    mitkNewMacro1Param(Self, const RuleIDType &);
    mitkNewMacro2Param(Self, const RuleIDType &, const std::string &);
    mitkNewMacro4Param(Self, const RuleIDType &, const std::string &, const std::string &, const std::string &);

    using RuleIDType = PropertyRelationRuleBase::RuleIDType;
    using RelationUIDType = PropertyRelationRuleBase::RelationUIDType;
    using RelationUIDVectorType = PropertyRelationRuleBase::RelationUIDVectorType;

    /** Returns an ID string that identifies the rule class */
    RuleIDType GetRuleID() const override;

    bool IsAbstract() const override;

    /** Returns a human readable string that can be used to describe the rule. Does not need to be unique.*/
    std::string GetDisplayName() const override;

    /** Returns a human readable string that can be used to describe the role of a source in context of the rule
     * instance.*/
    std::string GetSourceRoleName() const override;
    /** Returns a human readable string that can be used to describe the role of a destination in context of the rule
     * instance.*/
    std::string GetDestinationRoleName() const override;

    /** Pass through to base implementation of PropertyRelationRuleBase. See PropertyRelationRuleBase::connect documentation for more information. */
    RelationUIDType Connect(IPropertyOwner *source, const IPropertyProvider *destination) const;

  protected:
    GenericIDRelationRule(const RuleIDType &ruleIDTag);
    GenericIDRelationRule(const RuleIDType &ruleIDTag, const std::string &displayName);
    GenericIDRelationRule(const RuleIDType &ruleIDTag,
                          const std::string &displayName,
                          const std::string &sourceRole,
                          const std::string &destinationRole);
    ~GenericIDRelationRule() override = default;

    using InstanceIDType = PropertyRelationRuleBase::InstanceIDType;
    using InstanceIDVectorType = PropertyRelationRuleBase::InstanceIDVectorType;

    using DataRelationUIDVectorType = PropertyRelationRuleBase::DataRelationUIDVectorType;
    DataRelationUIDVectorType GetRelationUIDs_DataLayer(const IPropertyProvider* source,
      const IPropertyProvider* destination, const InstanceIDVectorType& instances_IDLayer) const override;

    void Connect_datalayer(IPropertyOwner *source,
                                   const IPropertyProvider *destination,
                                   const InstanceIDType &instanceID) const override;

    void Disconnect_datalayer(IPropertyOwner *source, const RelationUIDType& relationUID) const override;

    bool IsSupportedRuleID(const RuleIDType& ruleID) const override;

    itk::LightObject::Pointer InternalClone() const override;

  private:
    RuleIDType m_RuleIDTag;
    std::string m_DisplayName;
    std::string m_SourceRole;
    std::string m_DestinationRole;
  };

} // namespace mitk

#endif
