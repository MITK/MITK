/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegmentationPropertyHelper.h>

#include <mitkDICOMSegmentationConstants.h>
#include <mitkExceptionMacro.h>
#include <mitkIPropertyProvider.h>
#include <mitkProperties.h>
#include <mitkPropertyKeyPath.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcuid.h>

#include <map>
#include <set>
#include <vector>

namespace
{
  std::string DICOMTagKey(unsigned int group, unsigned int element)
  {
    return mitk::GeneratePropertyNameForDICOMTag(group, element);
  }

  void SetIfMissing(mitk::MultiLabelSegmentation* seg,
                    const std::string& key,
                    mitk::BaseProperty* value)
  {
    if (seg->GetProperty(key.c_str()).IsNull())
      seg->SetProperty(key.c_str(), value);
  }

  // Presence (not non-emptiness) is the check. Type-2 DICOM tags may be
  // intentionally empty; if Validate insisted on non-empty values, the
  // "set to empty on purpose" pattern would be impossible to express and
  // Complete would silently overwrite intentional emptiness with "UNKNOWN".
  bool HasProperty(const mitk::MultiLabelSegmentation* seg,
                   const std::string& key)
  {
    return seg->GetConstProperty(key).IsNotNull();
  }

  void AddSegmentationLevelMissing(std::vector<mitk::DICOMSegmentationPropertyHelper::MissingItem>& out,
                                   const std::string& description)
  {
    mitk::DICOMSegmentationPropertyHelper::MissingItem item;
    item.scope = mitk::DICOMSegmentationPropertyHelper::MissingItem::Scope::Segmentation;
    item.description = description;
    out.push_back(item);
  }

  void AddLabelMissing(std::vector<mitk::DICOMSegmentationPropertyHelper::MissingItem>& out,
                       mitk::Label::PixelType labelValue,
                       const std::string& description)
  {
    mitk::DICOMSegmentationPropertyHelper::MissingItem item;
    item.scope = mitk::DICOMSegmentationPropertyHelper::MissingItem::Scope::Label;
    item.identifier = std::to_string(labelValue);
    item.description = description;
    out.push_back(item);
  }

  struct SegmentationLevelRequirement
  {
    unsigned int group;
    unsigned int element;
    const char* description;
  };

  // Clinical Trial Series Module (carrying 0012,0071 / 0012,0050) and
  // Clinical Trial Subject Module (carrying 0012,0060) are OPTIONAL in
  // the SEG IOD. They are not listed here so Validate never demands a
  // value for them; the writer emits the tags only when the user has
  // explicitly populated the corresponding property.
  constexpr SegmentationLevelRequirement kSegmentationLevelTags[] = {
    {0x0008, 0x0060, "Modality (0008,0060)"},
    // Series Description is DICOM Type 3 (optional), not a SEG IOD
    // obligation. It is required here as an MITK branding invariant:
    // Complete always stamps "MITK Segmentation", so Validate never
    // blocks on it in practice.
    {0x0008, 0x103E, "Series Description (0008,103E)"},
    {0x0070, 0x0084, "Content Creator Name (0070,0084)"},
    {0x0010, 0x0010, "PatientName (0010,0010)"},
    {0x0010, 0x0020, "PatientID (0010,0020)"},
    {0x0020, 0x0010, "StudyID (0020,0010)"},
    {0x0020, 0x000D, "StudyInstanceUID (0020,000D)"},
    {0x0020, 0x000E, "SeriesInstanceUID (0020,000E)"},
    {0x0020, 0x0052, "FrameOfReferenceUID (0020,0052)"},
  };
}

std::string mitk::DICOMSegmentationPropertyHelper::MintSyntheticUID(const std::string& /*kind*/)
{
  // Today: DCMTK's site-rooted generator. When MITK gains a registered
  // organisation root with a documented "synth / unknown data" subspace,
  // this body becomes the only place that needs to change; the `kind`
  // argument is reserved for that future subspacing.
  char buffer[100] = {0};
  dcmGenerateUniqueIdentifier(buffer, SITE_INSTANCE_UID_ROOT);
  return std::string(buffer);
}

const std::string& mitk::DICOMSegmentationPropertyHelper::UnknownPatientName()
{
  static const std::string value = "UNKNOWN^UNKNOWN";
  return value;
}
const std::string& mitk::DICOMSegmentationPropertyHelper::UnknownPatientID()
{
  static const std::string value = "UNKNOWN";
  return value;
}
const std::string& mitk::DICOMSegmentationPropertyHelper::UnknownStudyID()
{
  static const std::string value = "UNKNOWN";
  return value;
}
const std::string& mitk::DICOMSegmentationPropertyHelper::UnknownContentCreatorName()
{
  static const std::string value = "UNKNOWN^UNKNOWN";
  return value;
}

namespace
{
  void CopyIfPresent(mitk::MultiLabelSegmentation* seg,
                     const mitk::IPropertyProvider* source,
                     unsigned int group, unsigned int element,
                     bool overwrite)
  {
    const auto key = mitk::GeneratePropertyNameForDICOMTag(group, element);
    const auto sourceProp = source->GetConstProperty(key);
    if (sourceProp.IsNull())
      return;
    if (!overwrite && seg->GetConstProperty(key).IsNotNull())
      return;
    seg->SetProperty(key.c_str(), sourceProp->Clone());
  }
}

void mitk::DICOMSegmentationPropertyHelper::InheritPatientFromSource(MultiLabelSegmentation* seg,
                                                                     const IPropertyProvider* source,
                                                                     bool overwrite)
{
  if (seg == nullptr)
    mitkThrow() << "DICOMSegmentationPropertyHelper::InheritPatientFromSource: seg must not be nullptr.";
  if (source == nullptr)
    mitkThrow() << "DICOMSegmentationPropertyHelper::InheritPatientFromSource: source must not be nullptr.";
  CopyIfPresent(seg, source, 0x0010, 0x0010, overwrite);
  CopyIfPresent(seg, source, 0x0010, 0x0020, overwrite);
}

void mitk::DICOMSegmentationPropertyHelper::InheritStudyFromSource(MultiLabelSegmentation* seg,
                                                                   const IPropertyProvider* source,
                                                                   bool overwrite)
{
  if (seg == nullptr)
    mitkThrow() << "DICOMSegmentationPropertyHelper::InheritStudyFromSource: seg must not be nullptr.";
  if (source == nullptr)
    mitkThrow() << "DICOMSegmentationPropertyHelper::InheritStudyFromSource: source must not be nullptr.";
  CopyIfPresent(seg, source, 0x0020, 0x0010, overwrite);
  CopyIfPresent(seg, source, 0x0020, 0x000d, overwrite);
}

void mitk::DICOMSegmentationPropertyHelper::InheritFrameOfReferenceFromSource(MultiLabelSegmentation* seg,
                                                                              const IPropertyProvider* source,
                                                                              bool overwrite)
{
  if (seg == nullptr)
    mitkThrow() << "DICOMSegmentationPropertyHelper::InheritFrameOfReferenceFromSource: seg must not be nullptr.";
  if (source == nullptr)
    mitkThrow() << "DICOMSegmentationPropertyHelper::InheritFrameOfReferenceFromSource: source must not be nullptr.";
  CopyIfPresent(seg, source, 0x0020, 0x0052, overwrite);
}

std::vector<mitk::DICOMSegmentationPropertyHelper::MissingItem>
mitk::DICOMSegmentationPropertyHelper::Validate(const MultiLabelSegmentation* seg)
{
  if (seg == nullptr)
    mitkThrow() << "DICOMSegmentationPropertyHelper::Validate: seg must not be nullptr.";

  std::vector<MissingItem> missing;

  for (const auto& req : kSegmentationLevelTags)
  {
    if (!HasProperty(seg, DICOMTagKey(req.group, req.element)))
      AddSegmentationLevelMissing(missing, req.description);
  }

  // Source-image relation is not checked here. ReferencedSeriesSequence is
  // type 1C in the SEG IOD (conditional on DerivationImageSequence) so a
  // source-less SEG is DICOM-legal; an ID-layer-only relation is also
  // fine for in-session use. The writer decides what to emit at export
  // time.

  // Algorithm Type is reported as missing when Undefined even though
  // Complete defaults it to MANUAL. The intent: a caller looking at
  // Validate's output sees the field they did not set, even though it
  // would be quietly auto-filled later.
  const auto labels = seg->GetLabels();
  for (const auto& label : labels)
  {
    if (label->GetValue() == Label::UNLABELED_VALUE)
      continue;

    const auto algoType = label->GetAlgorithmType();
    if (algoType == Label::AlgorithmType::Undefined)
      AddLabelMissing(missing, label->GetValue(), "Algorithm Type (0062,0008)");

    // SegmentAlgorithmName (0062,0009) is DICOM Type 1C: required only when
    // SegmentAlgorithmType is AUTOMATIC or SEMIAUTOMATIC. dcmqi correctly
    // omits the tag for MANUAL labels, so requiring a non-empty name
    // unconditionally would block a legitimate round trip on Validate.
    if ((algoType == Label::AlgorithmType::AUTOMATIC
         || algoType == Label::AlgorithmType::SEMIAUTOMATIC)
        && label->GetAlgorithmName().empty())
      AddLabelMissing(missing, label->GetValue(), "Algorithm Name (0062,0009)");

    // Tracking ID/UID (0062,0020/0062,0021) are Type 3 in the SEG IOD's
    // Segment Description Macro, so absence is DICOM-legal. Segmented
    // Property Category/Type (0062,0003/0062,000F) are Type 1, but the
    // writer fills them with an explicit "unknown" code when absent so
    // strict mode does not need to demand a user-supplied value here.
  }

  return missing;
}

std::vector<mitk::DICOMSegmentationPropertyHelper::MissingItem>
mitk::DICOMSegmentationPropertyHelper::Complete(MultiLabelSegmentation* seg,
                                                const CompletionOptions& options)
{
  if (seg == nullptr)
    mitkThrow() << "DICOMSegmentationPropertyHelper::Complete: seg must not be nullptr.";

  // Class invariants: Modality is fixed by the type, and the remaining
  // textuals are MITK branding that survive a later synthesis call
  // because every write is fill-only-if-missing. Identifying tags
  // (PatientName, PatientID, StudyID, ...) are deliberately NOT stamped
  // here so Initialize(templateImage) and InheritXxxFromSource can adopt
  // them from a real source.
  SetIfMissing(seg, DICOMTagKey(0x0008, 0x0060),
               TemporoSpatialStringProperty::New("SEG"));
  SetIfMissing(seg, DICOMTagKey(0x0008, 0x103E),
               TemporoSpatialStringProperty::New("MITK Segmentation"));
  SetIfMissing(seg, DICOMTagKey(0x0070, 0x0084),
               TemporoSpatialStringProperty::New("MITK"));

  // SeriesInstanceUID is the seg's own identity, never inherited from a
  // source image (the source's series UID belongs to the source). Mint
  // at construction so every seg carries a stable (0020,000E) from New()
  // onward, and strict-mode writes work without opting into synthetic
  // mode. Stability across the copy ctor holds: BaseData's copy ctor
  // clones the property list first, then Complete(this, {}) hits
  // SetIfMissing and no-ops on the cloned property.
  SetIfMissing(seg, DICOMTagKey(0x0020, 0x000E),
               TemporoSpatialStringProperty::New(DICOMSegmentationPropertyHelper::MintSyntheticUID("series")));

  if (options.synthesizeMissingIdentity)
  {
    SetIfMissing(seg, DICOMTagKey(0x0010, 0x0010),
                 TemporoSpatialStringProperty::New(UnknownPatientName()));
    SetIfMissing(seg, DICOMTagKey(0x0010, 0x0020),
                 TemporoSpatialStringProperty::New(UnknownPatientID()));
    SetIfMissing(seg, DICOMTagKey(0x0020, 0x0010),
                 TemporoSpatialStringProperty::New(UnknownStudyID()));
    SetIfMissing(seg, DICOMTagKey(0x0070, 0x0084),
                 TemporoSpatialStringProperty::New(UnknownContentCreatorName()));

    SetIfMissing(seg, DICOMTagKey(0x0020, 0x000D),
                 TemporoSpatialStringProperty::New(DICOMSegmentationPropertyHelper::MintSyntheticUID("study")));
    SetIfMissing(seg, DICOMTagKey(0x0020, 0x0052),
                 TemporoSpatialStringProperty::New(DICOMSegmentationPropertyHelper::MintSyntheticUID("for")));
  }

  if (options.deriveGeometryFromSegmentation)
  {
    // The seg's own per-frame geometry stands in for the absent source's
    // frame of reference. When synthesizeMissingIdentity also runs, the
    // earlier block already minted (0020,0052); SetIfMissing makes this a
    // no-op then. The block is kept so callers can derive a FoR without
    // committing to the full identity-placeholder set.
    SetIfMissing(seg, DICOMTagKey(0x0020, 0x0052),
                 TemporoSpatialStringProperty::New(DICOMSegmentationPropertyHelper::MintSyntheticUID("for")));
  }

  if (options.synthesizeMissingIdentity)
  {
    const auto labels = seg->GetLabels();
    for (const auto& label : labels)
    {
      if (label->GetValue() == Label::UNLABELED_VALUE)
        continue;
      if (label->GetAlgorithmType() == Label::AlgorithmType::Undefined)
        label->SetAlgorithmType(Label::AlgorithmType::MANUAL);
    }
  }

  return Validate(seg);
}

namespace
{
  // Identifying tags extracted from one source DICOM file. Empty fields
  // mean the file did not declare the tag; we drop entries without a
  // SOPInstanceUID because the rule's per-slice property has nothing to
  // record for them.
  struct LegacySourceFileTags
  {
    std::string sopInstanceUID;
    std::string sopClassUID;
    std::string seriesInstanceUID;
  };

  bool ReadIdentifyingTags(const std::string& path, LegacySourceFileTags& out)
  {
    DcmFileFormat ff;
    // Only the identifying header UIDs are needed; stop parsing before
    // PixelData so large CT/MR source files are not read in full during
    // legacy migration.
    if (ff.loadFileUntilTag(path.c_str(), EXS_Unknown, EGL_noChange,
                            DCM_MaxReadLength, ERM_autoDetect, DCM_PixelData).bad())
      return false;
    DcmDataset* dataset = ff.getDataset();
    if (dataset == nullptr)
      return false;

    OFString tmp;
    if (dataset->findAndGetOFString(DCM_SOPInstanceUID, tmp).good())
      out.sopInstanceUID = tmp.c_str();
    if (dataset->findAndGetOFString(DCM_SOPClassUID, tmp).good())
      out.sopClassUID = tmp.c_str();
    if (dataset->findAndGetOFString(DCM_SeriesInstanceUID, tmp).good())
      out.seriesInstanceUID = tmp.c_str();
    return !out.sopInstanceUID.empty();
  }
}

std::size_t mitk::DICOMSegmentationPropertyHelper::MigrateLegacyReferenceFilesToRelation(MultiLabelSegmentation* seg)
{
  if (seg == nullptr)
    mitkThrow() << "DICOMSegmentationPropertyHelper::MigrateLegacyReferenceFilesToRelation: seg must not be nullptr.";

  // Skip if any rule connection is already present: the seg is already
  // post-migration (loaded from a freshly written native file, or
  // produced in-session via Initialize / SetupDerivedSegmentation).
  // Re-running the migration would duplicate relations.
  if (!SegSourceImageRelationRule::GetSourceImageRelations(seg).empty())
    return 0;

  const auto filesProp = dynamic_cast<const StringLookupTableProperty*>(
    seg->GetProperty("referenceFiles").GetPointer());
  if (filesProp == nullptr)
    return 0;

  // GetValue here is the itkGetConstMacro form which returns the
  // StringLookupTable by value; capture the copy in a local so the
  // const reference to its inner std::map below does not dangle.
  const auto lookupCopy = filesProp->GetValue();
  const auto& lookup = lookupCopy.GetLookupTable();
  if (lookup.empty())
    return 0;

  // Slice index used for the per-slice TemporoSpatialString properties:
  // the StringLookupTable's keys are arbitrary integer IDs (typically
  // 1-based file order) and do not necessarily align with seg frame
  // indices. Slice ordering of legacy referenceFiles is also not
  // guaranteed to follow ImagePositionPatient order. The pragmatic
  // choice is to use the lookup-table key directly as the slice index;
  // a malformed sequence shows up as a sparse property but does not
  // corrupt the relation. Downstream writes that need per-frame source
  // association will fall back to the SEG IOD's type-1C absence path
  // if the property does not cover a given frame.
  //
  // Source files that carry no SeriesInstanceUID coalesce under the
  // empty-string series key below, i.e. they collapse into a single
  // relation with an empty source series. Rare (source files normally
  // carry a Series UID) and degrades gracefully (the relation still
  // loads); not special-cased here.
  std::map<std::string, std::map<int, LegacySourceFileTags>> bySeries;
  std::size_t resolved = 0;
  std::size_t unresolved = 0;
  for (const auto& [key, filePath] : lookup)
  {
    LegacySourceFileTags tags;
    if (!ReadIdentifyingTags(filePath, tags))
    {
      MITK_WARN << "MigrateLegacyReferenceFilesToRelation: cannot read \""
                << filePath << "\"; source-image relation will skip it.";
      ++unresolved;
      continue;
    }
    bySeries[tags.seriesInstanceUID][key] = std::move(tags);
    ++resolved;
  }

  if (resolved == 0)
  {
    MITK_WARN << "MigrateLegacyReferenceFilesToRelation: none of the "
              << lookup.size() << " referenceFiles entries could be opened; "
              << "leaving the property in place for potential later retry.";
    return 0;
  }

  std::size_t connectionsEstablished = 0;
  for (const auto& [seriesUID, perSlice] : bySeries)
  {
    auto perSliceInstance = TemporoSpatialStringProperty::New();
    auto perSliceClass = TemporoSpatialStringProperty::New();
    for (const auto& [sliceIndex, tags] : perSlice)
    {
      perSliceInstance->SetValue(0, static_cast<TemporoSpatialStringProperty::IndexValueType>(sliceIndex),
                                 tags.sopInstanceUID);
      if (!tags.sopClassUID.empty())
      {
        perSliceClass->SetValue(0, static_cast<TemporoSpatialStringProperty::IndexValueType>(sliceIndex),
                                tags.sopClassUID);
      }
    }

    auto rule = SegSourceImageRelationRule::New();
    try
    {
      rule->Connect(seg, perSliceInstance, perSliceClass, seriesUID);
      ++connectionsEstablished;
    }
    catch (const std::exception& e)
    {
      MITK_WARN << "MigrateLegacyReferenceFilesToRelation: Connect failed for "
                << "source series \"" << seriesUID << "\": " << e.what();
    }
  }

  // Drop referenceFiles only when the migration fully succeeded: at least one
  // relation was established AND every entry resolved (unresolved == 0). If any
  // entry failed to resolve (e.g. an offline network share), keep the property
  // so the legacy record survives for a potential later recovery.
  //
  // Known limitation: once any relation exists on the seg, the early-out at the
  // top of this function suppresses re-migration, so a partially-resolved seg is
  // not retried on a later load even if the missing files become available - the
  // still-unresolved entries keep their referenceFiles record but are never
  // converted to relations. Accepted for now (partial source availability is
  // rare); revisit if it proves a problem in practice.
  if (connectionsEstablished > 0 && unresolved == 0)
  {
    seg->GetPropertyList()->RemoveProperty("referenceFiles");
  }

  return connectionsEstablished;
}
