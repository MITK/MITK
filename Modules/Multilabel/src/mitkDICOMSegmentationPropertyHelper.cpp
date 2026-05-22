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
#include <mitkStringProperty.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <dcmtk/dcmdata/dcuid.h>

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

  // Encapsulates the UID root choice so a future MITK-registered root can
  // be introduced by editing this one function. Today, synthesis falls
  // back to DCMTK's dcmGenerateUniqueIdentifier, which mints UIDs under
  // OFFIS's site root (SITE_INSTANCE_UID_ROOT) and guarantees uniqueness;
  // those UIDs are DICOM-compliant but are not visually identifiable as
  // MITK placeholders. When MITK gains a registered organisation root,
  // route MintSynthUID through a custom DCMTK call (e.g. pass the MITK
  // root as the prefix argument) without touching the call sites.
  std::string MintSynthUID(const std::string& /*kind*/)
  {
    char buffer[100] = {0};
    dcmGenerateUniqueIdentifier(buffer, SITE_INSTANCE_UID_ROOT);
    return std::string(buffer);
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

  constexpr SegmentationLevelRequirement kSegmentationLevelTags[] = {
    {0x0008, 0x0060, "Modality (0008,0060)"},
    {0x0008, 0x103E, "Series Description (0008,103E)"},
    {0x0070, 0x0084, "Content Creator Name (0070,0084)"},
    {0x0012, 0x0071, "Clinical Trial Series ID (0012,0071)"},
    {0x0012, 0x0050, "Clinical Trial Time Point ID (0012,0050)"},
    {0x0012, 0x0060, "Clinical Trial Coordinating Center Name (0012,0060)"},
    {0x0010, 0x0010, "PatientName (0010,0010)"},
    {0x0010, 0x0020, "PatientID (0010,0020)"},
    {0x0020, 0x0010, "StudyID (0020,0010)"},
    {0x0020, 0x000D, "StudyInstanceUID (0020,000D)"},
    {0x0020, 0x000E, "SeriesInstanceUID (0020,000E)"},
    {0x0020, 0x0052, "FrameOfReferenceUID (0020,0052)"},
  };
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
const std::string& mitk::DICOMSegmentationPropertyHelper::UnknownClinicalTrialSeriesID()
{
  static const std::string value = "UNKNOWN";
  return value;
}
const std::string& mitk::DICOMSegmentationPropertyHelper::UnknownClinicalTrialTimePointID()
{
  static const std::string value = "UNKNOWN";
  return value;
}
const std::string& mitk::DICOMSegmentationPropertyHelper::UnknownClinicalTrialCoordinatingCenterName()
{
  static const std::string value = "UNKNOWN";
  return value;
}
const std::string& mitk::DICOMSegmentationPropertyHelper::UnknownBodyPartExamined()
{
  static const std::string value;
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

    if (label->GetAlgorithmType() == Label::AlgorithmType::Undefined)
      AddLabelMissing(missing, label->GetValue(), "Algorithm Type (0062,0008)");

    if (label->GetAlgorithmName().empty())
      AddLabelMissing(missing, label->GetValue(), "Algorithm Name (0062,0009)");

    if (!label->GetSegmentedPropertyCategory().has_value())
      AddLabelMissing(missing, label->GetValue(),
        "Segmented Property Category (0062,0003)");

    if (!label->GetSegmentedPropertyType().has_value())
      AddLabelMissing(missing, label->GetValue(),
        "Segmented Property Type (0062,000F)");

    // Presence (not non-emptiness) is the check. The DICOM SEG reader
    // deliberately stamps empty tracking ID/UID to suppress MITK's
    // automatic UID generation; those labels carry the property in the
    // map with an empty value and must be treated as "set, not missing".
    if (!label->HasTrackingID())
      AddLabelMissing(missing, label->GetValue(), "Tracking ID (0062,0020)");

    if (!label->HasTrackingUID())
      AddLabelMissing(missing, label->GetValue(), "Tracking UID (0062,0021)");
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
  SetIfMissing(seg, DICOMTagKey(0x0012, 0x0071),
               TemporoSpatialStringProperty::New(UnknownClinicalTrialSeriesID()));

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
    SetIfMissing(seg, DICOMTagKey(0x0012, 0x0050),
                 TemporoSpatialStringProperty::New(UnknownClinicalTrialTimePointID()));
    SetIfMissing(seg, DICOMTagKey(0x0012, 0x0060),
                 TemporoSpatialStringProperty::New(UnknownClinicalTrialCoordinatingCenterName()));

    SetIfMissing(seg, DICOMTagKey(0x0020, 0x000D),
                 TemporoSpatialStringProperty::New(MintSynthUID("study")));
    SetIfMissing(seg, DICOMTagKey(0x0020, 0x000E),
                 TemporoSpatialStringProperty::New(MintSynthUID("series")));
    SetIfMissing(seg, DICOMTagKey(0x0020, 0x0052),
                 TemporoSpatialStringProperty::New(MintSynthUID("for")));
  }

  if (options.deriveGeometryFromSegmentation)
  {
    // The seg's own per-frame geometry stands in for the absent source's
    // frame of reference.
    SetIfMissing(seg, DICOMTagKey(0x0020, 0x0052),
                 TemporoSpatialStringProperty::New(MintSynthUID("for")));
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
