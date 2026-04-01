/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSourceImageRelationRule_h
#define mitkSourceImageRelationRule_h

#include <mitkPropertyRelationRuleBase.h>
#include <mitkImage.h>

namespace mitk
{
  /** \brief Relation rule for referencing an image as the source of a destination entity.
   *
   * This rule class can be used for relations that reference an image as source for
   * a destination entity (e.g. an image that is used to generate the relation source).
   *
   * The ID-layer is supported like for GenericIDRelations, so it can be used for all
   * ID-based relations between PropertyProviders that also implement the Identifiable interface.
   *
   * In addition, the rule uses the data-layer to deduce/define relations. For this layer
   * it uses properties compliant to DICOM. Thus (1) the information is stored in a DICOM
   * Source Image Sequence item (0x0008,0x2112) and (2) the destination must have properties
   * DICOM SOP Instance UID (0x0008,0x0018) and DICOM SOP Class UID (0x0008,0x0016). If the
   * destination does not have these properties, no connection can be made on the data-layer.
   *
   * \remark PropertyRelationRules and DICOM use the term "source" differently. The DICOM
   * source (image) equals the PropertyRelationRule destination. This is due to an inverted
   * relation direction. In the context of this rule interface, derived data is the source
   * and points to the original image it derives from. In DICOM, this referenced original
   * image would be called the source image (as the name of this class suggests).
   *
   * In order to use this class for different relation types (DICOM would call them purposes),
   * the purposeTag is used. It must be specified when creating a rule instance. The purposeTag
   * will be used as suffix for the rule ID and therefore allows creating specific and
   * distinguishable rule instances based on this class.
   *
   * One may also specify the display name and the role names of the instance. If not specified,
   * the default values are used (display name: "<purposeTag> relation", source role name:
   * "derived data", destination role name: "source image").
   *
   * \sa PropertyRelationRuleBase
   * \sa GenericIDRelationRule
   */
  class MITKCORE_EXPORT SourceImageRelationRule : public mitk::PropertyRelationRuleBase
  {
  public:
    mitkClassMacro(SourceImageRelationRule, PropertyRelationRuleBase);
    itkNewMacro(Self);
    mitkNewMacro1Param(Self, const RuleIDType &);
    mitkNewMacro2Param(Self, const RuleIDType &, const std::string &);
    mitkNewMacro4Param(Self, const RuleIDType &, const std::string &, const std::string &, const std::string &);

    using RuleIDType = PropertyRelationRuleBase::RuleIDType;
    using RelationUIDType = PropertyRelationRuleBase::RelationUIDType;
    using RelationUIDVectorType = PropertyRelationRuleBase::RelationUIDVectorType;

    /** \brief Return an ID string that identifies the rule class. */
    RuleIDType GetRuleID() const override;

    /** \brief Return whether this rule is abstract (i.e. has no purpose tag). */
    bool IsAbstract() const override;

    /** \brief Return a human-readable string describing the rule. Does not need to be unique. */
    std::string GetDisplayName() const override;

    /** \brief Return a human-readable string describing the role of a source in the context of this rule. */
    std::string GetSourceRoleName() const override;

    /** \brief Return a human-readable string describing the role of a destination in the context of this rule. */
    std::string GetDestinationRoleName() const override;

    /** \brief Check whether the given property provider qualifies as a destination for this rule. */
    bool IsDestinationCandidate(const IPropertyProvider *owner) const override;

    /** \brief Connect two images using this relation rule.
     *
     * \remark The destination must specify DICOM SOP Instance UID (0x0008,0x0018) and
     * DICOM SOP Class UID (0x0008,0x0016) in order to establish a connection on the data layer.
     * \param[in] source The derived image (relation source).
     * \param[in] destination The original image (relation destination / DICOM source image).
     * \return The UID of the newly created relation.
     */
    RelationUIDType Connect(Image *source, const Image *destination) const;

  protected:
    /** \brief Default constructor. Creates an abstract rule with no purpose tag. */
    SourceImageRelationRule();

    /** \brief Construct a rule with a specific purpose tag.
     * \param[in] purposeTag The purpose tag used as suffix for the rule ID.
     */
    SourceImageRelationRule(const RuleIDType &purposeTag);

    /** \brief Construct a rule with a purpose tag and display name.
     * \param[in] purposeTag The purpose tag used as suffix for the rule ID.
     * \param[in] displayName Human-readable name for the rule.
     */
    SourceImageRelationRule(const RuleIDType &purposeTag, const std::string &displayName);

    /** \brief Construct a rule with all customizable parameters.
     * \param[in] purposeTag The purpose tag used as suffix for the rule ID.
     * \param[in] displayName Human-readable name for the rule.
     * \param[in] sourceRole Human-readable name for the source role.
     * \param[in] destinationRole Human-readable name for the destination role.
     */
    SourceImageRelationRule(const RuleIDType &purposeTag,
                          const std::string &displayName,
                          const std::string &sourceRole,
                          const std::string &destinationRole);

    ~SourceImageRelationRule() override = default;

    using InstanceIDType = PropertyRelationRuleBase::InstanceIDType;
    using InstanceIDVectorType = PropertyRelationRuleBase::InstanceIDVectorType;

    /** \brief Get reference sequence indices from the DICOM Source Image Sequence (0008,2112).
     *
     * Returns a vector of (sequence index, ruleID) pairs that refer to the given destination
     * (or all if no destination is passed), have a supported RuleID, and are not already
     * covered by ignoreInstances.
     */
    std::vector<std::pair<size_t, std::string> > GetReferenceSequenceIndices(const IPropertyProvider * source,
      const IPropertyProvider* destination = nullptr, InstanceIDVectorType ignoreInstances = {}) const;

    using DataRelationUIDVectorType = PropertyRelationRuleBase::DataRelationUIDVectorType;
    virtual DataRelationUIDVectorType GetRelationUIDs_DataLayer(const IPropertyProvider* source,
      const IPropertyProvider* destination, const InstanceIDVectorType& instances_IDLayer) const override;

    void Connect_datalayer(IPropertyOwner *source,
                                   const IPropertyProvider *destination,
                                   const InstanceIDType &instanceID) const override;

    void Disconnect_datalayer(IPropertyOwner *source, const RelationUIDType& relationUID) const override;

    bool IsSupportedRuleID(const RuleIDType& ruleID) const override;

    SourceImageRelationRule(const SourceImageRelationRule &other);

    mitkCloneMacro(Self);

    /** \brief Prepare a new reference to an image on the data layer.
     *
     * Generates an unused and valid sequence item index for the passed source and sets a
     * relationUID property to block the instance ID. The instance ID is returned.
     *
     * \remark This method is guarded by a class-wide mutex to avoid race conditions when
     * rules are used concurrently.
     */
    PropertyKeyPath::ItemSelectionIndex CreateNewSourceImageSequenceItem(IPropertyOwner *source) const;

    /** \brief Generate a rule ID string from the given purpose tag. */
    std::string GenerateRuleID(const std::string& purpose) const;

  private:
    RuleIDType m_PurposeTag;
    std::string m_DisplayName;
    std::string m_SourceRole;
    std::string m_DestinationRole;
  };

} // namespace mitk

#endif
