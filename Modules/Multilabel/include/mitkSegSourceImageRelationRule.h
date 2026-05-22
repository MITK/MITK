/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegSourceImageRelationRule_h
#define mitkSegSourceImageRelationRule_h

#include <mitkSourceImageRelationRule.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <MitkMultilabelExports.h>

namespace mitk
{
  class MultiLabelSegmentation;

  /**
   * \brief Relation rule for referencing a segmentation to a source-image derivations.
   *
   * This rule class can be used for relations that reference an image as source for
   * a segmentation.
   *
   * The ID-layer is supported like for GenericIDRelations, so it can be used for all
   * ID-based relations between PropertyProviders that also implement the Identifiable interface.
   *
   * In addition, the rule uses the data-layer to deduce/define relations. For this layer
   * it uses properties compliant to DICOM. In addition to the DICOM data used by
   * SourceImageRelationRule this class also adds the source SeriesInstanceUID (0020,000e)
   * to the data captured by the base SourceImageRelationRule. The series UID is stored on
   * the rule itself rather than as a top-level seg property so a single SEG can
   * cleanly reference multiple source series, mirroring DICOM's
   * ReferencedSeriesSequence (0008,1115) one item per series shape.
   *
   * The canonical DICOM-SEG-source purpose tag is type-bound: callers
   * acquire an instance via New() and the rule applies the tag itself.
   * This prevents producers from drifting away from the canonical value
   * and lets diagnostic code recover the value by querying the type.
   *
   * \remark PropertyRelationRules and DICOM use the term "source" differently. The DICOM
   * source (image) equals the PropertyRelationRule destination. This is due to an inverted
   * relation direction. In the context of this rule interface, derived data is the source
   * and points to the original image it derives from. In DICOM, this referenced original
   * image would be called the source image (as the name of this class suggests).
   *
   * \sa SourceImageRelationRule
   * \sa DICOMSegmentationPropertyHelper
   */
  class MITKMULTILABEL_EXPORT SegSourceImageRelationRule : public SourceImageRelationRule
  {
  public:
    mitkClassMacro(SegSourceImageRelationRule, SourceImageRelationRule);
    itkNewMacro(Self);

    /**
     * \brief Returns the canonical DICOM Code Value used in the SEG's
     *        Source Image Sequence Purpose Of Reference Code.
     *
     * Used at tag-comparison boundaries (DCMQI, tests, diagnostics) so the
     * canonical value lives in exactly one place rather than as duplicated
     * string literals scattered through callers.
     */
    static const std::string& CanonicalPurposeTag();

    /**
     * \brief One seg-source relation as the DICOM SEG writer needs to see it.
     *
     * The writer turns each entry into one item of the SEG's top-level
     * ReferencedSeriesSequence (0008,1115). Per-slice UID properties are
     * exposed as pointers because they are cloned from the source at
     * Connect time and the writer reads them in place.
     */
    struct SourceImageRelation
    {
      std::string relationUID;
      std::string sourceSeriesInstanceUID;
      TemporoSpatialStringProperty::ConstPointer instanceUIDsPerSlice;
      TemporoSpatialStringProperty::ConstPointer classUIDsPerSlice;
    };

    /**
     * \brief Enumerates the seg-source relations established on seg.
     *
     * Hides the per-relation storage layout from the writer (which would
     * otherwise have to know the rule's MITK.Relations.<instanceID>.*
     * conventions to assemble the writer-side SourceImageRelation entries).
     *
     * \pre seg must be a valid pointer.
     */
    static std::vector<SourceImageRelation>
      GetSourceImageRelations(const MultiLabelSegmentation* seg);

    /**
     * \brief Connects a segmentation to a source represented as an
     *        IPropertyProvider.
     *
     * The base SourceImageRelationRule offers a typed Connect(Image*, Image*)
     * wrapper. The reader path needs to connect against an ad-hoc provider
     * (a PropertyList carrying SOPInstanceUID / SOPClassUID / SeriesInstanceUID
     * synthesized from the SEG file's metadata) that is not an Image, hence
     * this overload. Sources without DICOM identifying tags still establish
     * an ID-layer relation; the data layer is captured only when the provider
     * carries SOP Instance / Class / Series UIDs.
     *
     * \pre seg must be a valid pointer.
     * \pre sourceProvider must be a valid pointer.
     */
    RelationUIDType Connect(MultiLabelSegmentation* seg,
                            const IPropertyProvider* sourceProvider) const;

    /**
     * \brief Static convenience that constructs an instance of this rule
     *        and connects seg to source_image.
     *
     * Use this from call sites that just want to establish a single
     * relation; the instance method is for callers that already hold a
     * rule and want to make multiple connections through it.
     *
     * Captures only the relation: per-source SOP Instance / SOP Class /
     * Series Instance UIDs. The seg's own identifying tags
     * (PatientName, StudyInstanceUID, FrameOfReferenceUID, ...) are not
     * touched here. Use DICOMSegmentationPropertyHelper's functions for that.
     *
     * \pre seg and source_image must be valid pointers.
     */
    static RelationUIDType Connect(MultiLabelSegmentation* seg,
                                   const Image* source_image);

  protected:
    SegSourceImageRelationRule();
    SegSourceImageRelationRule(const SegSourceImageRelationRule& other);

    /**
     * \brief Extends the base data-layer Connect by capturing the source
     *        SeriesInstanceUID per relation.
     *
     * Storing the series UID on the relation (rather than as a top-level
     * seg property) keeps Disconnect self-contained: removing a relation
     * removes its series UID through normal virtual dispatch, without the
     * caller having to know the extension exists.
     */
    void Connect_datalayer(IPropertyOwner* source,
                           const IPropertyProvider* destination,
                           const InstanceIDType& instanceID) const override;

    /**
     * \brief Removes this rule's SourceSeriesInstanceUID extension before
     *        delegating to the base.
     *
     * The base resolves the instance ID by relation UID, which only works
     * before its own per-relation properties are gone. The extension must
     * therefore be cleaned up first.
     */
    void Disconnect_datalayer(IPropertyOwner* source,
                              const RelationUIDType& relationUID) const override;

    mitkCloneMacro(Self);
  };
} // namespace mitk

#endif
