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
  One may also specify the display name and the role names of the instance. If not speficied the default values
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

    /** Is called if a instance ID cannot be deduced on the ID-layer.
    Implement this method to check which existing relation(s) as Connected_Data exists between
    both passed instances. If the passed instances have no
    explicit relation of type Connected_Data, an empty vector will be returned.
    @remark Per definition of property relation rules only 0 or 1 instance should be found for one provider
    pair and rule. But the data layer may be ambiguous and there for muliple relation instances of the rule instance
    could match. The implementation of this function should report all relation instances. The calling function
    will take care of this violation.
    @pre source must be a pointer to a valid IPropertyProvider instance.
    @pre destination must be a pointer to a valid IPropertyProvider instance.*/
    InstanceIDVectorType GetInstanceID_datalayer(const IPropertyProvider *source,
                                                         const IPropertyProvider *destination) const override;

    /** Is called by HasRelation() if no relation of type Connected_ID (GetInstanceID_IDLayer()) or
    Connected_Data (GetInstanceID_datalayer()) is evident.
      Implement this method to deduce if the passed instances have a relation of type
      Implicit_Data.
      @pre source must be a pointer to a valid IPropertyProvider instance.
      @pre destination must be a pointer to a valid IPropertyProvider instance.
      */
    bool HasImplicitDataRelation(const IPropertyProvider *source,
                                         const IPropertyProvider *destination) const override;

    /**Is called by Connect() to ensure that source has correctly set properties to resemble
    the relation on the data layer. This means that the method should set the properties that describe
    and encode the relation on the data layer (data-layer-specific relation properties).
    If the passed instance are already connected, the old settings should be
    overwritten. Connect() will ensure that source and destination are valid pointers.
    @param instanceID is the ID for the relation instance that should be connected. Existance of the relation instance
    is ensured.
    @pre source must be a valid instance.
    @pre destination must be a valid instance.*/
    void Connect_datalayer(IPropertyOwner *source,
                                   const IPropertyProvider *destination,
                                   const InstanceIDType &instanceID) const override;

    /**This method is called by Disconnect() to remove all properties of the relation from the source that
    are set by Connect_datalayer().
    @remark All RII-properties of this relation will removed by Disconnect() after this method call.
    If the relationUID is not part of the source. Nothing will be changed. Disconnect() ensures that source is a valid
    pointer if called.
    @remark Disconnect() ensures that sourece is valid and only invokes if instance exists.*/
    void Disconnect_datalayer(IPropertyOwner *source, const InstanceIDType &instanceID) const override;

    virtual bool IsSupportedRuleID(const RuleIDType& ruleID) const override;

    itk::LightObject::Pointer InternalClone() const override;

  private:
    RuleIDType m_RuleIDTag;
    std::string m_DisplayName;
    std::string m_SourceRole;
    std::string m_DestinationRole;
  };

} // namespace mitk

#endif
