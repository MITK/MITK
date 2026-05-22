/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSegmentationPropertyHelper_h
#define mitkDICOMSegmentationPropertyHelper_h

#include <mitkDICOMTag.h>
#include <mitkLabel.h>
#include <mitkLabelSetImage.h>

#include <MitkMultilabelExports.h>

#include <string>
#include <vector>

namespace mitk
{
  class IPropertyProvider;

  /**
   * \brief Helpers around the DICOM SEG property contract on MultiLabelSegmentation.
   *
   * Validate and Complete are deliberately split into a pure-inspection and
   * a single-mutator pair so callers can ask "what is missing" without
   * triggering side effects, and synthesize only at points where mutation
   * is acceptable (typically just before write). Complete uses fill-only-
   * if-missing semantics so that branding defaults stamped by Complete
   * itself with default options (called from the MultiLabelSegmentation
   * constructors) coexist with later synthesis without being overwritten
   * by generic "UNKNOWN" placeholders.
   *
   * \sa SegSourceImageRelationRule
   * \sa MultiLabelSegmentation
   */
  class MITKMULTILABEL_EXPORT DICOMSegmentationPropertyHelper
  {
  public:
    /** \brief One item missing from the DICOM SEG property contract on a seg. */
    struct MissingItem
    {
      enum class Scope
      {
        Segmentation,
        Group,
        Label
      };

      Scope scope;
      /** Group index or label value, depending on scope. */
      std::string identifier;
      std::string description;
    };

    /** \brief Synthesis options for Complete. Flags are independent and additive. */
    struct CompletionOptions
    {
      /**
       * \brief Fill missing patient- and study-level identifying tags from
       *        the placeholder constants below and mint UIDs for
       *        StudyInstanceUID / SeriesInstanceUID / FrameOfReferenceUID
       *        under the MITK synth namespace.
       *
       * Reserved for explicit write-time synthesis, not for ambient mutation:
       * minting UIDs is one-way (the seg's identity is fixed afterwards).
       * Also defaults each label's AlgorithmType to MANUAL when it is
       * Undefined; the per-label fields the SEG IOD requires (Algorithm
       * Name, Segmented Property Category/Type, Tracking ID/UID) are
       * never fabricated by Complete.
       */
      bool synthesizeMissingIdentity = false;

      /**
       * \brief Mint a FrameOfReferenceUID even when no real source is
       *        available, so the seg is self-contained in its spatial frame.
       *
       * Separate from synthesizeMissingIdentity because callers may want to
       * stamp a frame-of-reference for in-house tooling without committing
       * to the broader patient/study placeholder set.
       */
      bool deriveGeometryFromSegmentation = false;
    };

    /**
     * \brief Reports what is missing from the seg's DICOM SEG contract
     *        without mutating it.
     *
     * Empty result means the seg is writable in strict mode without
     * further synthesis.
     *
     * \pre seg must be a valid pointer.
     */
    static std::vector<MissingItem> Validate(const MultiLabelSegmentation* seg);

    /**
     * \brief Stamps MITK-specific branding defaults and applies the
     *        opt-in synthesis flags.
     *
     * Called from MultiLabelSegmentation's constructors with default
     * options to stamp the class invariants (Modality="SEG" and
     * MITK-branded series description / content creator / clinical
     * trial series ID). Identifying tags (PatientName, PatientID,
     * StudyID, FrameOfReferenceUID, ...) are NOT stamped unless
     * synthesizeMissingIdentity is set, so a later Initialize(template)
     * or InheritXxxFromSource can still adopt source values.
     *
     * Never overwrites a property that is already set. Never fabricates
     * per-instance ReferencedSOPInstanceUIDs because those would lie about
     * data that exists in the world (unlike top-level placeholder UIDs
     * which describe the seg itself).
     *
     * \pre seg must be a valid pointer.
     */
    static std::vector<MissingItem> Complete(MultiLabelSegmentation* seg,
                                             const CompletionOptions& options);

    /**
     * \brief Copies PatientName (0010,0010) and PatientID (0010,0020) from
     *        source to seg.
     *
     * Useful when the seg is derived from an image in a known patient and
     * should appear under that patient in PACS / downstream tooling. Not
     * entailed by establishing a source-image relation: a seg can derive
     * from a source while declaring a different patient, but that is
     * unusual and out of scope here.
     *
     * Default semantics: overwrite. Calling InheritPatientFromSource means
     * "treat the source's patient identity as authoritative on the seg".
     * Pass overwrite=false to keep any values already set on the seg.
     *
     * \param seg       The segmentation to mutate.
     * \param source    Property provider to read the identity from.
     * \param overwrite If true (default), overwrites existing values on the
     *                  seg; if false, fills only missing slots.
     * \pre seg must be a valid pointer.
     * \pre source must be a valid pointer.
     */
    static void InheritPatientFromSource(MultiLabelSegmentation* seg,
                                         const IPropertyProvider* source,
                                         bool overwrite = true);

    /**
     * \brief Copies StudyID (0020,0010) and StudyInstanceUID (0020,000d)
     *        from source to seg.
     *
     * Useful when the seg should appear under the same study as the
     * source in PACS. Not entailed by source-image derivation: cross-study
     * derivations are valid DICOM but uncommon.
     *
     * Default semantics: overwrite. See InheritPatientFromSource.
     *
     * \pre seg must be a valid pointer.
     * \pre source must be a valid pointer.
     */
    static void InheritStudyFromSource(MultiLabelSegmentation* seg,
                                       const IPropertyProvider* source,
                                       bool overwrite = true);

    /**
     * \brief Copies FrameOfReferenceUID (0020,0052) from source to seg.
     *
     * Useful when the seg lives in the source's spatial frame, which is
     * typical when the seg's geometry was initialized from the source.
     * Not entailed by source-image derivation: a seg can derive from a
     * source while living in a different frame of reference (e.g. a
     * resampled seg).
     *
     * Default semantics: overwrite. See InheritPatientFromSource.
     *
     * \pre seg must be a valid pointer.
     * \pre source must be a valid pointer.
     */
    static void InheritFrameOfReferenceFromSource(MultiLabelSegmentation* seg,
                                                  const IPropertyProvider* source,
                                                  bool overwrite = true);

    // Placeholder constants used by Complete's synthesis path. Exposed so
    // tests, downstream tooling, and consumers reading a MITK-written SEG
    // can distinguish placeholders from real-world data by string match
    // against one canonical source.
    static const std::string& UnknownPatientName();
    static const std::string& UnknownPatientID();
    static const std::string& UnknownStudyID();
    static const std::string& UnknownContentCreatorName();
    static const std::string& UnknownClinicalTrialSeriesID();
    static const std::string& UnknownClinicalTrialTimePointID();
    static const std::string& UnknownClinicalTrialCoordinatingCenterName();
    static const std::string& UnknownBodyPartExamined();
  };
}

#endif
