/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSegSourceImageRelationRule.h>

#include <mitkExceptionMacro.h>
#include <mitkLabelSetImage.h>
#include <mitkPropertyKeyPath.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>

namespace
{
  const std::string SOURCE_SERIES_UID_KEY = "SourceSeriesInstanceUID";
  const std::string SOURCE_IMAGE_SEQUENCE_ITEM_KEY = "SourceImageSequenceItem";

  std::string MakeRelationScopedKey(const std::string& instanceID, const std::string& leaf)
  {
    return mitk::PropertyKeyPathToPropertyName(
      mitk::PropertyRelationRuleBase::GetRootKeyPath()
        .AddElement(instanceID)
        .AddElement(leaf));
  }
}

const std::string& mitk::SegSourceImageRelationRule::CanonicalPurposeTag()
{
  // DICOM Code Value "121322" (scheme "DCM", "Source image for image
  // processing operation"). Matches what DCMQI emits, so SEG files written
  // and re-read by MITK compare equal at the tag level.
  static const std::string tag = "121322";
  return tag;
}

mitk::SegSourceImageRelationRule::SegSourceImageRelationRule()
  : SourceImageRelationRule(CanonicalPurposeTag(),
                            "DICOM SEG source image relation",
                            "DICOM SEG",
                            "source image")
{
}

mitk::SegSourceImageRelationRule::SegSourceImageRelationRule(const SegSourceImageRelationRule& other)
  : SourceImageRelationRule(other)
{
}

mitk::SegSourceImageRelationRule::RelationUIDType
mitk::SegSourceImageRelationRule::Connect(MultiLabelSegmentation* seg, const Image* sourceImage)
{
  if (seg == nullptr)
    mitkThrow() << "SegSourceImageRelationRule::Connect: seg must not be nullptr.";
  if (sourceImage == nullptr)
    mitkThrow() << "SegSourceImageRelationRule::Connect: source_image must not be nullptr.";

  auto rule = SegSourceImageRelationRule::New();
  return rule->Connect(seg, static_cast<const IPropertyProvider*>(sourceImage));
}

mitk::SegSourceImageRelationRule::RelationUIDType
mitk::SegSourceImageRelationRule::Connect(MultiLabelSegmentation* seg,
                                          const IPropertyProvider* sourceProvider) const
{
  if (seg == nullptr)
    mitkThrow() << "SegSourceImageRelationRule::Connect: seg must not be nullptr.";
  if (sourceProvider == nullptr)
    mitkThrow() << "SegSourceImageRelationRule::Connect: sourceProvider must not be nullptr.";

  // The Image*-typed Connect on SourceImageRelationRule cannot accept an
  // ad-hoc IPropertyProvider (the reader path), so the base IPropertyOwner-
  // typed Connect is the only reachable entry. The Image*-typed wrapper adds
  // no behaviour of its own, only a type constraint.
  return PropertyRelationRuleBase::Connect(seg, sourceProvider);
}

mitk::SegSourceImageRelationRule::RelationUIDType
mitk::SegSourceImageRelationRule::Connect(MultiLabelSegmentation* seg,
                                          TemporoSpatialStringProperty* instanceUIDsPerSlice,
                                          TemporoSpatialStringProperty* classUIDsPerSlice,
                                          const std::string& sourceSeriesInstanceUID) const
{
  if (seg == nullptr)
    mitkThrow() << "SegSourceImageRelationRule::Connect: seg must not be nullptr.";
  if (instanceUIDsPerSlice == nullptr || classUIDsPerSlice == nullptr)
    mitkThrow() << "SegSourceImageRelationRule::Connect: per-slice UID properties must not be nullptr.";

  // Assemble the DICOM-tag-keyed provider the data layer expects: SOPInstance
  // (0008,0018) and SOPClass (0008,0016) per slice, plus the source
  // SeriesInstanceUID (0020,000e) when known. Centralised here so the reader
  // and the legacy-migration paths share one definition of this shape.
  auto provider = PropertyList::New();
  provider->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0018).c_str(), instanceUIDsPerSlice);
  provider->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0016).c_str(), classUIDsPerSlice);
  if (!sourceSeriesInstanceUID.empty())
  {
    provider->SetProperty(GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str(),
                          TemporoSpatialStringProperty::New(sourceSeriesInstanceUID));
  }

  return this->Connect(seg, provider.GetPointer());
}

void mitk::SegSourceImageRelationRule::Connect_datalayer(IPropertyOwner* source,
                                                         const IPropertyProvider* destination,
                                                         const InstanceIDType& instanceID) const
{
  Superclass::Connect_datalayer(source, destination, instanceID);

  // Sources without DICOM tags (e.g. NIfTI loads) leave seriesProp null;
  // the base in that case already skipped its data-layer work too, so the
  // relation is ID-layer-only and the SEG writer will emit no
  // ReferencedSeriesSequence entry for it.
  const auto seriesProp = destination->GetConstProperty(GeneratePropertyNameForDICOMTag(0x0020, 0x000e));
  if (seriesProp.IsNotNull())
  {
    source->SetProperty(MakeRelationScopedKey(instanceID, SOURCE_SERIES_UID_KEY),
                        seriesProp->Clone());
  }
}

void mitk::SegSourceImageRelationRule::Disconnect_datalayer(IPropertyOwner* source,
                                                            const RelationUIDType& relationUID) const
{
  // GetInstanceIDByRelationUID relies on the base's per-relation properties,
  // which Superclass::Disconnect_datalayer below will remove. Resolve and
  // clean up our extension first.
  try
  {
    const auto instanceID = this->GetInstanceIDByRelationUID(source, relationUID);
    source->RemoveProperty(MakeRelationScopedKey(instanceID, SOURCE_SERIES_UID_KEY));
  }
  catch (...)
  {
    // Pure data-layer relation (no ID-layer instance): our extension is keyed
    // by instance ID and was never written, so nothing to clean up.
  }

  Superclass::Disconnect_datalayer(source, relationUID);
}

std::vector<mitk::SegSourceImageRelationRule::SourceImageRelation>
mitk::SegSourceImageRelationRule::GetSourceImageRelations(const MultiLabelSegmentation* seg)
{
  if (seg == nullptr)
    mitkThrow() << "SegSourceImageRelationRule::GetSourceImageRelations: seg must not be nullptr.";

  std::vector<SourceImageRelation> result;

  auto rule = SegSourceImageRelationRule::New();
  const auto relationUIDs = rule->GetExistingRelations(seg);
  result.reserve(relationUIDs.size());

  for (const auto& relationUID : relationUIDs)
  {
    SourceImageRelation entry;
    entry.relationUID = relationUID;

    InstanceIDType instanceID;
    try
    {
      instanceID = rule->GetInstanceIDByRelationUID(seg, relationUID);
    }
    catch (...)
    {
      // The relation exists only on the data layer (no ID-layer instance);
      // our extension and the per-slice properties are both keyed by
      // instance ID, so without one there is nothing further to surface.
      result.push_back(entry);
      continue;
    }

    const auto seriesProp = seg->GetConstProperty(MakeRelationScopedKey(instanceID, SOURCE_SERIES_UID_KEY));
    if (seriesProp.IsNotNull())
      entry.sourceSeriesInstanceUID = seriesProp->GetValueAsString();

    const auto sequenceIndexProp = seg->GetConstProperty(
      MakeRelationScopedKey(instanceID, SOURCE_IMAGE_SEQUENCE_ITEM_KEY));
    if (sequenceIndexProp.IsNotNull())
    {
      // SourceImageSequenceItem is written by the base rule as std::to_string(index)
      // under normal operation; a malformed property (corrupted persistence,
      // manual edit) would otherwise propagate std::invalid_argument /
      // std::out_of_range out of pybind11 boundaries. Skip the per-slice
      // lookup on parse failure but keep the relation in the result.
      std::uint64_t sequenceIndex = 0;
      bool sequenceIndexValid = false;
      try
      {
        sequenceIndex = std::stoull(sequenceIndexProp->GetValueAsString());
        sequenceIndexValid = true;
      }
      catch (const std::exception& e)
      {
        MITK_WARN << "SegSourceImageRelationRule::GetSourceImageRelations: "
                  << "malformed SourceImageSequenceItem property for instance "
                  << instanceID << ": " << e.what();
      }

      if (sequenceIndexValid)
      {
        PropertyKeyPath refInstanceUIDPath;
        refInstanceUIDPath.AddElement("DICOM").AddElement("0008")
          .AddSelection("2112", sequenceIndex).AddElement("0008").AddElement("1155");
        const auto instanceUIDProp = seg->GetConstProperty(PropertyKeyPathToPropertyName(refInstanceUIDPath));
        entry.instanceUIDsPerSlice = dynamic_cast<const TemporoSpatialStringProperty*>(instanceUIDProp.GetPointer());

        PropertyKeyPath refClassUIDPath;
        refClassUIDPath.AddElement("DICOM").AddElement("0008")
          .AddSelection("2112", sequenceIndex).AddElement("0008").AddElement("1150");
        const auto classUIDProp = seg->GetConstProperty(PropertyKeyPathToPropertyName(refClassUIDPath));
        entry.classUIDsPerSlice = dynamic_cast<const TemporoSpatialStringProperty*>(classUIDProp.GetPointer());
      }
    }

    result.push_back(entry);
  }

  return result;
}
