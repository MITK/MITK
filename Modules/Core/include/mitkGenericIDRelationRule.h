/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGenericIDRelationRule_h
#define mitkGenericIDRelationRule_h

#include <mitkPropertyRelationRuleBase.h>

namespace mitk
{
  /**
   * \brief Relation rule for ID-only relations between IIdentifiable property providers.
   *
   * This rule class is used for relations that are defined solely on the ID-layer,
   * with no connection on the Data-layer. It works for all ID-based relations between
   * PropertyProviders that implement the IIdentifiable interface.
   *
   * The \c ruleIDTag parameter, specified at construction time, serves as a suffix for
   * the rule ID ("IDRelation_<ruleIDTag>") and allows creating distinguishable rule
   * instances. When \c ruleIDTag is empty, the rule is abstract and can detect any
   * generic ID relation but cannot create new connections.
   *
   * Default role names (when not specified):
   * - Display name: "<ruleIDTag> relation"
   * - Source role: "source of <ruleIDTag> relation"
   * - Destination role: "destination of <ruleIDTag> relation"
   *
   * \ingroup DataManagement
   *
   * \sa PropertyRelationRuleBase
   * \sa SourceImageRelationRule
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

    /**
     * \brief Get the rule ID string ("IDRelation_<ruleIDTag>").
     * \return The rule ID string.
     */
    RuleIDType GetRuleID() const override;

    /**
     * \brief Check whether this rule instance is abstract.
     *
     * The rule is abstract when the ruleIDTag is empty.
     *
     * \return \c true if the ruleIDTag is empty (abstract rule), \c false otherwise.
     */
    bool IsAbstract() const override;

    /**
     * \brief Get a human-readable display name for the rule.
     * \return The display name string.
     */
    std::string GetDisplayName() const override;

    /**
     * \brief Get a human-readable description of the source role.
     * \return The source role name string.
     */
    std::string GetSourceRoleName() const override;

    /**
     * \brief Get a human-readable description of the destination role.
     * \return The destination role name string.
     */
    std::string GetDestinationRoleName() const override;

    /**
     * \brief Connect two property providers with an ID-based relation.
     *
     * Creates a new relation or updates an existing one between source and
     * destination. The destination must implement IIdentifiable.
     *
     * \param[in,out] source The source property owner to add the relation to.
     * \param[in] destination The destination property provider to relate to.
     * \return The UID of the created or updated relation.
     *
     * \pre \p source must be a valid instance.
     * \pre \p destination must be a valid instance.
     * \pre The rule must not be abstract.
     *
     * \sa PropertyRelationRuleBase::Connect
     */
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

    GenericIDRelationRule(const GenericIDRelationRule &other);

    mitkCloneMacro(Self);

  private:
    RuleIDType m_RuleIDTag;
    std::string m_DisplayName;
    std::string m_SourceRole;
    std::string m_DestinationRole;
  };

} // namespace mitk

#endif
