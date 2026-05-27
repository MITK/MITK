/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegmentationIO.h>

#include <mitkDICOMSegIOMimeTypes.h>
#include <mitkDICOMSegmentationConstants.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMIOHelper.h>
#include <mitkDICOMProperty.h>
#include <mitkDICOMSegmentationPropertyHelper.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkLocaleSwitch.h>
#include <mitkPlaneGeometry.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkTemporoSpatialStringProperty.h>


// itk
#include <itkThresholdImageFilter.h>

// dcmqi
#include <dcmqi/Itk2DicomConverter.h>
#include <dcmqi/Dicom2ItkConverterBase.h>
#include <dcmqi/JSONSegmentationMetaInformationHandler.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmfg/fgderimg.h>
#include <dcmtk/dcmfg/fginterface.h>
#include <dcmtk/dcmfg/fgtypes.h>
#include <dcmtk/dcmseg/segdoc.h>

#include <iomanip>
#include <limits>
#include <sstream>

namespace
{
  // Writer IO options. Names are user-facing in the file IO dialog, so the
  // wording is the same shape as the existing Multilabel writer options
  // ("Save strategy" / "Instance value").
  constexpr const char *OPTION_SYNTHESIS_MODE = "Strict / synthetic mode";
  constexpr const char *OPTION_SYNTHESIS_MODE_STRICT = "strict";
  constexpr const char *OPTION_SYNTHESIS_MODE_SYNTHETIC = "synthetic";
  constexpr const char *OPTION_ENCODING = "Segmentation encoding";
  constexpr const char *OPTION_ENCODING_LABELMAP = "labelmap";
  constexpr const char *OPTION_ENCODING_BINARY = "binary";

  // Resolve an enum-style writer option to its selected string value.
  // The option's default is registered as std::vector<std::string> (the
  // shape MITK's QmitkFileReaderWriterOptionsWidget renders as a combo
  // box); once the user picks an entry the Any holds a std::string. When
  // the writer is invoked without a UI (programmatic IOUtil::Save with
  // empty options or unset enum), the Any still holds the registered
  // vector and the first entry is the default by convention. ToString()
  // on a vector<string> returns "[a,b,...]", so a direct string compare
  // against the expected choice silently misses the default - handle
  // both shapes explicitly here.
  std::string ResolveEnumOption(const mitk::IFileWriter::Options &options,
                                const std::string &name,
                                const std::string &fallback)
  {
    const auto it = options.find(name);
    if (it == options.end() || it->second.Empty())
      return fallback;
    if (it->second.Type() == typeid(std::string))
      return us::any_cast<std::string>(it->second);
    if (it->second.Type() == typeid(std::vector<std::string>))
    {
      const auto &vec = us::ref_any_cast<std::vector<std::string>>(it->second);
      return vec.empty() ? fallback : vec.front();
    }
    return fallback;
  }

  // Format one MissingItem to human-readable for exception messages. The
  // structured shape (Scope, identifier, description) is preserved for
  // diagnostic tooling; the formatter is the textual face the user sees.
  std::string FormatMissingItem(const mitk::DICOMSegmentationPropertyHelper::MissingItem &m)
  {
    std::ostringstream out;
    switch (m.scope)
    {
      case mitk::DICOMSegmentationPropertyHelper::MissingItem::Scope::Segmentation:
        out << "[segmentation] " << m.description;
        break;
      case mitk::DICOMSegmentationPropertyHelper::MissingItem::Scope::Group:
        out << "[group " << m.identifier << "] " << m.description;
        break;
      case mitk::DICOMSegmentationPropertyHelper::MissingItem::Scope::Label:
        out << "[label " << m.identifier << "] " << m.description;
        break;
    }
    return out.str();
  }

  // Decide whether the writer can apply dcmqi's useLabelIDAsSegmentNumber.
  // dcmqi requires the label values within one group to be a monotonic
  // 1..N with no gaps; the writer mirrors that precondition rather than
  // letting dcmqi fail late. When the labels do not satisfy it the writer
  // falls back to dcmqi's default 1..N-in-encounter-order numbering and
  // the per-segment labelID in the metainfo handler is what carries the
  // MITK label value across a round trip.
  bool LabelsAreMonotonicOneToN(const mitk::MultiLabelSegmentation *seg, unsigned int layer)
  {
    const auto labelValues = seg->GetLabelValuesByGroup(layer);
    if (labelValues.empty())
      return false;

    std::vector<mitk::MultiLabelSegmentation::LabelValueType> sorted(labelValues.begin(), labelValues.end());
    std::sort(sorted.begin(), sorted.end());

    for (size_t i = 0; i < sorted.size(); ++i)
    {
      if (sorted[i] != static_cast<mitk::MultiLabelSegmentation::LabelValueType>(i + 1))
        return false;
    }
    return true;
  }

  // Copy one top-level DICOM tag from seg's property list into dst if the
  // seg carries the value. dcmqi reads Patient / Study / FoR off the
  // first dcmDatasets entry via importHierarchy and reads
  // SeriesInstanceUID off the same entry for the SEG's top-level
  // ReferencedSeriesSequence, so the source DcmItem must carry the
  // values back even when MITK already has them on the seg itself.
  bool CopyTopLevelDICOMTagToItem(const mitk::MultiLabelSegmentation *seg,
                                  unsigned int group,
                                  unsigned int element,
                                  const DcmTagKey &targetTag,
                                  DcmItem &dst)
  {
    const auto key = mitk::GeneratePropertyNameForDICOMTag(group, element);
    const auto prop = seg->GetConstProperty(key);
    if (prop.IsNull())
      return false;
    const std::string value = prop->GetValueAsString();
    return dst.putAndInsertString(targetTag, value.c_str()).good();
  }

  // Stamp the Patient / Study / Frame-of-Reference identifying tags from
  // the seg onto the source DcmItem dcmqi treats as dcmDatasets[0].
  // The tag set mirrors what DICOMQIPropertyHelper::DeriveDICOMSourceProperties
  // historically copied from the source image, so a round trip preserves
  // the same identifying information that the legacy path would have.
  void StampSegIdentityOnSourceItem(const mitk::MultiLabelSegmentation *seg, DcmItem &dst)
  {
    // Patient module
    CopyTopLevelDICOMTagToItem(seg, 0x0010, 0x0010, DCM_PatientName, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0010, 0x0020, DCM_PatientID, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0010, 0x0030, DCM_PatientBirthDate, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0010, 0x0040, DCM_PatientSex, dst);
    // Study module
    CopyTopLevelDICOMTagToItem(seg, 0x0020, 0x000D, DCM_StudyInstanceUID, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0020, 0x0010, DCM_StudyID, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0008, 0x0020, DCM_StudyDate, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0008, 0x0030, DCM_StudyTime, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0008, 0x0050, DCM_AccessionNumber, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0008, 0x0090, DCM_ReferringPhysicianName, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0008, 0x1030, DCM_StudyDescription, dst);
    // Frame of Reference module
    CopyTopLevelDICOMTagToItem(seg, 0x0020, 0x0052, DCM_FrameOfReferenceUID, dst);
    CopyTopLevelDICOMTagToItem(seg, 0x0020, 0x1040, DCM_PositionReferenceIndicator, dst);
  }

  // Format coordinates to the maximum precision a double round-trips at.
  // DICOM DS VR allows up to 16 significant digits; the default
  // std::ostream precision (6) loses sub-mm geometry information for
  // images far from origin or with sub-mm spacing, and downstream
  // tooling that round-trips through PlaneGeometry's metric arithmetic
  // can land on an off-by-one slice index.
  std::ostringstream MakeDicomDecimalStream()
  {
    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    return out;
  }

  // Stamp ImageOrientationPatient on dst from the seg's group geometry.
  // dcmqi does not read IOP from individual source datasets for the
  // slice mapping (it uses IPP only) but the value belongs on a
  // well-formed source DICOM image and downstream consumers expect it.
  void StampImageOrientationOnSourceItem(const mitk::Image *groupImage, DcmItem &dst)
  {
    if (groupImage == nullptr || groupImage->GetGeometry() == nullptr)
      return;
    auto row = groupImage->GetGeometry()->GetAxisVector(0);
    auto col = groupImage->GetGeometry()->GetAxisVector(1);
    row.Normalize();
    col.Normalize();
    auto iop = MakeDicomDecimalStream();
    iop << row[0] << "\\" << row[1] << "\\" << row[2] << "\\"
        << col[0] << "\\" << col[1] << "\\" << col[2];
    dst.putAndInsertString(DCM_ImageOrientationPatient, iop.str().c_str());
  }

  // Format an IPP value for one slice of a group image. PlaneGeometry's
  // origin is the patient-coordinate position of the slice (MITK uses a
  // corner-of-first-voxel convention; DICOM's ImagePositionPatient is
  // defined as the centre of the upper-left voxel). dcmqi parses the
  // formatted IPP back into a Point3D and resolves the matching seg
  // frame via PlaneGeometry's TransformPhysicalPointToIndex on the
  // label image, so the requirement on this string is "round-trips
  // through the seg's own geometry," not exact string equality.
  std::string FormatIPPForSlice(const mitk::Image *groupImage,
                                mitk::TemporoSpatialStringProperty::IndexValueType sliceIndex)
  {
    const auto *slicedGeometry = groupImage->GetSlicedGeometry();
    if (slicedGeometry == nullptr)
      return {};
    const auto *plane = slicedGeometry->GetPlaneGeometry(static_cast<int>(sliceIndex));
    if (plane == nullptr)
      return {};
    const auto origin = plane->GetOrigin();
    auto out = MakeDicomDecimalStream();
    out << origin[0] << "\\" << origin[1] << "\\" << origin[2];
    return out.str();
  }

  // Read SegmentsOverlap (0062,0013) and decide if the reader must assume
  // overlapping segments. Used only by the binary read branch: each segment
  // image arrives separately and the reader has to choose between one shared
  // MITK group (non-overlap) and one group per segment (overlap). The labelmap
  // (Sup 243) branch ignores the flag because labelmap encoding already
  // forbids overlap within a single SEG file.
  bool ShouldAssumeOverlappingSegments(DcmDataset &dataSet)
  {
    OFString overlapValue;
    if (dataSet.findAndGetOFString(DCM_SegmentsOverlap, overlapValue).bad())
      return true;
    // DCM permits NO, YES, UNDEFINED; lower/mixed case forms are tolerated
    // here to stay robust against non-compliant producers.
    return "NO" != overlapValue && "no" != overlapValue && "No" != overlapValue;
  }

  // Resolve a display name for a DICOM SEG segment. Mirrors the historical
  // fallback chain: SegmentLabel -> SegmentedPropertyType code meaning (with
  // optional modifier) -> string form of the numeric segment id.
  OFString DeriveLabelName(dcmqi::SegmentAttributes &segmentAttribute)
  {
    OFString labelName = segmentAttribute.getSegmentLabel();
    if (!labelName.empty())
      return labelName;

    if (segmentAttribute.getSegmentedPropertyTypeCodeSequence() != nullptr)
    {
      segmentAttribute.getSegmentedPropertyTypeCodeSequence()->getCodeMeaning(labelName);
      if (segmentAttribute.getSegmentedPropertyTypeModifierCodeSequence() != nullptr)
      {
        OFString modifier;
        segmentAttribute.getSegmentedPropertyTypeModifierCodeSequence()->getCodeMeaning(modifier);
        labelName.append(" (").append(modifier).append(")");
      }
      return labelName;
    }

    return OFString(std::to_string(segmentAttribute.getLabelID()).c_str());
  }

  // One source-image reference seen on one SEG frame. Frame index is the
  // SEG's own frame numbering (0-based) and is later used as the slice
  // index in the TemporoSpatialStringProperty handed to the relation rule.
  struct SegSourceFrameRef
  {
    Uint32 frameIndex;
    std::string sopInstanceUID;
    std::string sopClassUID;
  };

  // A set of per-frame source references that share one source series.
  // The DICOM SEG IOD's top-level ReferencedSeriesSequence has exactly one
  // item per source series, so this 1:1 corresponds to one connect call on
  // the relation rule.
  struct SegSourceSeriesGroup
  {
    std::string seriesInstanceUID;
    std::vector<SegSourceFrameRef> frames;
  };

  // Frames that carry no derivation reference are silently skipped: the
  // SEG IOD makes the Derivation Image FG type 1C, so absence is DICOM-legal
  // and simply means "no source image is recorded for that frame."
  // Parameter is non-const because DCMTK's getFunctionalGroups and
  // getNumberOfFrames have no const overload.
  std::vector<SegSourceFrameRef> CollectPerFrameSourceRefs(DcmSegmentation& segDoc)
  {
    std::vector<SegSourceFrameRef> result;
    FGInterface& fgInterface = segDoc.getFunctionalGroups();
    const size_t numFrames = segDoc.getNumberOfFrames();

    for (size_t f = 0; f < numFrames; ++f)
    {
      OFBool isPerFrame = OFFalse;
      auto* fg = fgInterface.get(static_cast<Uint32>(f), DcmFGTypes::EFG_DERIVATIONIMAGE, isPerFrame);
      auto* derImg = OFstatic_cast(FGDerivationImage*, fg);
      if (derImg == nullptr)
        continue;

      OFVector<DerivationImageItem*>& derItems = derImg->getDerivationImageItems();
      for (auto* derItem : derItems)
      {
        if (derItem == nullptr)
          continue;
        OFVector<SourceImageItem*>& srcItems = derItem->getSourceImageItems();
        for (auto* srcItem : srcItems)
        {
          if (srcItem == nullptr)
            continue;
          OFString sopInstance;
          OFString sopClass;
          // ImageSOPInstanceReferenceMacro inherits the two getters from
          // SOPInstanceReferenceMacro; the macro is held by value on the
          // SourceImageItem.
          srcItem->getImageSOPInstanceReference().getReferencedSOPInstanceUID(sopInstance);
          srcItem->getImageSOPInstanceReference().getReferencedSOPClassUID(sopClass);
          if (sopInstance.empty())
            continue;
          result.push_back({static_cast<Uint32>(f), sopInstance.c_str(), sopClass.c_str()});
        }
      }
    }

    return result;
  }

  // An empty result means the SEG has no top-level series references at
  // all (DICOM-legal type 1C absence) and the per-series grouping below
  // degenerates to "no relations." Parameter is non-const because DCMTK's
  // findAndGet* are not const-overloaded.
  std::map<std::string, std::set<std::string>>
    CollectSourceInstancesBySeries(DcmDataset& dataset)
  {
    std::map<std::string, std::set<std::string>> result;
    DcmSequenceOfItems* refSeriesSeq = nullptr;
    if (dataset.findAndGetSequence(DCM_ReferencedSeriesSequence, refSeriesSeq).bad()
        || refSeriesSeq == nullptr)
      return result;

    for (unsigned long i = 0; i < refSeriesSeq->card(); ++i)
    {
      DcmItem* item = refSeriesSeq->getItem(i);
      if (item == nullptr)
        continue;

      OFString seriesUID;
      if (item->findAndGetOFString(DCM_SeriesInstanceUID, seriesUID).bad() || seriesUID.empty())
        continue;

      auto& instanceSet = result[seriesUID.c_str()];

      DcmSequenceOfItems* refInstSeq = nullptr;
      if (item->findAndGetSequence(DCM_ReferencedInstanceSequence, refInstSeq).bad()
          || refInstSeq == nullptr)
        continue;

      for (unsigned long j = 0; j < refInstSeq->card(); ++j)
      {
        DcmItem* instItem = refInstSeq->getItem(j);
        if (instItem == nullptr)
          continue;
        OFString sopInstance;
        if (instItem->findAndGetOFString(DCM_ReferencedSOPInstanceUID, sopInstance).good()
            && !sopInstance.empty())
        {
          instanceSet.insert(sopInstance.c_str());
        }
      }
    }

    return result;
  }

  // Invert the per-frame refs against the series map. Frames whose source
  // SOPInstanceUID matches no known series are warned about and dropped
  // from any group: this indicates a malformed SEG, not a reader-side
  // failure mode, so the seg still loads but the unresolved frame is not
  // attributed to a relation.
  std::vector<SegSourceSeriesGroup> GroupPerFrameRefsBySeries(
    const std::vector<SegSourceFrameRef>& frameRefs,
    const std::map<std::string, std::set<std::string>>& seriesToInstances)
  {
    std::vector<SegSourceSeriesGroup> groups;
    if (seriesToInstances.empty())
      return groups;

    std::map<std::string, size_t> seriesToGroupIndex;
    for (const auto& frameRef : frameRefs)
    {
      const std::string* matchedSeries = nullptr;
      for (const auto& [seriesUID, instanceSet] : seriesToInstances)
      {
        if (instanceSet.count(frameRef.sopInstanceUID) > 0)
        {
          matchedSeries = &seriesUID;
          break;
        }
      }

      if (matchedSeries == nullptr)
      {
        MITK_WARN << "DICOM SEG references source SOP Instance " << frameRef.sopInstanceUID
                  << " on frame " << frameRef.frameIndex
                  << " but the SEG's ReferencedSeriesSequence does not declare it; "
                  << "skipping frame from source-image relation population.";
        continue;
      }

      auto [it, inserted] = seriesToGroupIndex.try_emplace(*matchedSeries, groups.size());
      if (inserted)
      {
        SegSourceSeriesGroup g;
        g.seriesInstanceUID = *matchedSeries;
        groups.push_back(std::move(g));
      }
      groups[it->second].frames.push_back(frameRef);
    }

    return groups;
  }

  // The PropertyList is an ad-hoc IPropertyProvider built only to feed
  // the rule's instance Connect overload; it has no lifetime beyond this
  // scope.
  void PopulateSourceImageRelations(mitk::MultiLabelSegmentation& seg,
                                    const std::vector<SegSourceSeriesGroup>& groups)
  {
    if (groups.empty())
      return;

    auto rule = mitk::SegSourceImageRelationRule::New();

    for (const auto& group : groups)
    {
      auto perSliceInstance = mitk::TemporoSpatialStringProperty::New();
      auto perSliceClass = mitk::TemporoSpatialStringProperty::New();
      for (const auto& frameRef : group.frames)
      {
        // Single time step in the SEG IOD; frame -> slice index 1:1 as
        // described on SegSourceFrameRef.
        perSliceInstance->SetValue(0, frameRef.frameIndex, frameRef.sopInstanceUID);
        // Mirror the migration helper's shape: only stamp SOPClass when
        // it is non-empty, so GetAvailableSlices(0) reports a consistent
        // cardinality across both upstream paths.
        if (!frameRef.sopClassUID.empty())
          perSliceClass->SetValue(0, frameRef.frameIndex, frameRef.sopClassUID);
      }

      auto provider = mitk::PropertyList::New();
      provider->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0018).c_str(),
                            perSliceInstance);
      provider->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0016).c_str(),
                            perSliceClass);
      provider->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str(),
                            mitk::TemporoSpatialStringProperty::New(group.seriesInstanceUID));

      rule->Connect(&seg, provider.GetPointer());
    }
  }

  // Build the source-image DcmItems dcmqi expects from the seg's
  // SegSourceImageRelationRule properties and its own per-frame geometry.
  //
  // One DcmItem per (relation, slice) entry. Each item carries the source
  // SOPInstance UID and SOPClass UID from the rule's per-slice properties,
  // and an ImagePositionPatient string derived from the seg's own geometry
  // at that slice index. The first emitted item additionally stamps
  // Patient / Study / FrameOfReferenceUID and SeriesInstanceUID from the
  // seg's own DICOM-tag properties because dcmqi reads Patient / Study /
  // FoR off dcmDatasets[0] via importHierarchy and uses dcmDatasets[0]'s
  // SeriesInstanceUID for the SEG's top-level ReferencedSeriesSequence.
  //
  // Relations whose per-slice properties are null are silently skipped
  // (ID-layer-only relations carry no per-frame UIDs and have nothing
  // to contribute on this path). Slices whose SOPInstanceUID or formatted
  // IPP is empty are skipped for the same reason — there is no usable
  // per-frame source reference to emit.
  //
  // Items are returned as unique_ptr so the caller controls their
  // lifetime. The dcmqi call needs a vector of raw DcmItem pointers; the
  // caller materialises that view before invoking the converter.
  //
  // \pre seg must be a valid pointer.
  // \pre layer must be a valid group index of seg.
  std::vector<std::unique_ptr<DcmItem>>
  BuildSourceItemsFromProperties(const mitk::MultiLabelSegmentation *seg, unsigned int layer)
  {
    std::vector<std::unique_ptr<DcmItem>> result;
    if (seg == nullptr)
      return result;

    const auto *groupImage = seg->GetGroupImage(layer);
    if (groupImage == nullptr)
      return result;

    const auto relations = mitk::SegSourceImageRelationRule::GetSourceImageRelations(seg);
    if (relations.empty())
      return result;

    // dcmqi treats dcmDatasets[0] as the identity anchor: importHierarchy
    // pulls Patient / Study / FrameOfReferenceUID off it, and the SEG's
    // top-level ReferencedSeriesSequence uses its SeriesInstanceUID.
    // Subsequent items only need to carry SOPInstance / SOPClass / IPP so
    // the slice mapping resolves. dcmqi today emits one
    // ReferencedSeriesSequence item using the first dataset's series UID,
    // so a multi-source MITK seg lands as one series on the wire (known
    // dcmqi limitation).
    bool isFirst = true;
    for (const auto &relation : relations)
    {
      if (relation.instanceUIDsPerSlice.IsNull() || relation.classUIDsPerSlice.IsNull())
        continue;

      const auto slices = relation.instanceUIDsPerSlice->GetAvailableSlices(0);
      for (const auto slice : slices)
      {
        const auto sopInstance = relation.instanceUIDsPerSlice->GetValue(0, slice);
        if (sopInstance.empty())
          continue;
        const auto sopClass = relation.classUIDsPerSlice->GetValue(0, slice);
        const auto ipp = FormatIPPForSlice(groupImage, slice);
        if (ipp.empty())
          continue;

        auto item = std::make_unique<DcmItem>();
        item->putAndInsertString(DCM_SOPInstanceUID, sopInstance.c_str());
        if (!sopClass.empty())
          item->putAndInsertString(DCM_SOPClassUID, sopClass.c_str());
        item->putAndInsertString(DCM_ImagePositionPatient, ipp.c_str());

        if (isFirst)
        {
          if (!relation.sourceSeriesInstanceUID.empty())
            item->putAndInsertString(DCM_SeriesInstanceUID, relation.sourceSeriesInstanceUID.c_str());
          StampSegIdentityOnSourceItem(seg, *item);
          StampImageOrientationOnSourceItem(groupImage, *item);
          isFirst = false;
        }

        result.push_back(std::move(item));
      }
    }

    return result;
  }

  // Build a synthetic per-slice source item set when the seg has no
  // SegSourceImageRelationRule connection to draw from. Used by the
  // writer's synthetic mode as a safety net for producers that did not
  // attach a real source; strict mode throws instead.
  //
  // Rule:
  //  - FrameOfReferenceUID is shared with the seg (truthful: the synthetic
  //    source lives in the same spatial frame the seg does).
  //  - SeriesInstanceUID is freshly minted (distinct from the seg's own
  //    series) so the SEG's top-level ReferencedSeriesSequence does not
  //    collide with the SEG's own series identity.
  //  - SOPInstanceUID is freshly minted per slice.
  //  - SOPClassUID is "Secondary Capture Image Storage"
  //    (1.2.840.10008.5.1.4.1.1.7) — honest "synthetic, non-primary
  //    modality" semantics rather than impersonating a real modality.
  //  - IPP / IOP come from the seg's own per-frame geometry, matching
  //    the property-driven path so dcmqi resolves the slice mapping
  //    identically.
  //  - Patient / Study identity are stamped from the seg by
  //    StampSegIdentityOnSourceItem.
  //
  // UID minting routes through DICOMSegmentationPropertyHelper::
  // MintSyntheticUID so a future MITK-rooted "synth" subnamespace
  // lands in one place.
  std::vector<std::unique_ptr<DcmItem>>
  BuildSyntheticSourceItems(const mitk::MultiLabelSegmentation *seg, unsigned int layer)
  {
    std::vector<std::unique_ptr<DcmItem>> result;
    if (seg == nullptr)
      return result;

    const auto *groupImage = seg->GetGroupImage(layer);
    if (groupImage == nullptr)
      return result;

    const auto *slicedGeometry = groupImage->GetSlicedGeometry();
    if (slicedGeometry == nullptr)
      return result;
    const unsigned int sliceCount = slicedGeometry->GetSlices();
    if (sliceCount == 0)
      return result;

    constexpr const char *SECONDARY_CAPTURE_SOP_CLASS = "1.2.840.10008.5.1.4.1.1.7";
    const std::string syntheticSeriesUID =
      mitk::DICOMSegmentationPropertyHelper::MintSyntheticUID("source-series");

    bool isFirst = true;
    for (unsigned int slice = 0; slice < sliceCount; ++slice)
    {
      const auto ipp = FormatIPPForSlice(groupImage, slice);
      if (ipp.empty())
        continue;

      auto item = std::make_unique<DcmItem>();
      item->putAndInsertString(DCM_SOPInstanceUID,
        mitk::DICOMSegmentationPropertyHelper::MintSyntheticUID("source-instance").c_str());
      item->putAndInsertString(DCM_SOPClassUID, SECONDARY_CAPTURE_SOP_CLASS);
      item->putAndInsertString(DCM_ImagePositionPatient, ipp.c_str());

      if (isFirst)
      {
        item->putAndInsertString(DCM_SeriesInstanceUID, syntheticSeriesUID.c_str());
        StampSegIdentityOnSourceItem(seg, *item);
        StampImageOrientationOnSourceItem(groupImage, *item);
        isFirst = false;
      }

      result.push_back(std::move(item));
    }

    return result;
  }
}

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk
{
  DICOMSegmentationIO::DICOMSegmentationIO()
    : AbstractFileIO(MultiLabelSegmentation::GetStaticNameOfClass(),
      mitk::MitkDICOMSEGIOMimeTypes::DICOMSEG_MIMETYPE_NAME(),
      "DICOM Segmentation")
  {
    AbstractFileWriter::SetRanking(10);
    AbstractFileReader::SetRanking(10);

    // The Options-as-vector-of-strings idiom encodes the available choices
    // in declaration order; the first entry is the default. Strict /
    // labelmap are placed first so the defaults are "synthesise nothing,
    // emit Sup 243 labelmap SEG".
    Options writerOptions;
    writerOptions[OPTION_SYNTHESIS_MODE] = std::vector<std::string>{
      OPTION_SYNTHESIS_MODE_STRICT, OPTION_SYNTHESIS_MODE_SYNTHETIC};
    writerOptions[OPTION_ENCODING] = std::vector<std::string>{
      OPTION_ENCODING_LABELMAP, OPTION_ENCODING_BINARY};
    this->AbstractFileWriter::SetDefaultOptions(writerOptions);

    this->RegisterService();
  }

  IFileIO::ConfidenceLevel DICOMSegmentationIO::GetWriterConfidenceLevel() const
  {
    if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
      return Unsupported;

    const auto *input = dynamic_cast<const MultiLabelSegmentation *>(this->GetInput());
    if (input == nullptr)
      return Unsupported;

    if (input->GetDimension() != 3)
    {
      MITK_INFO << "DICOM segmentation writer is tested only with 3D images, sorry.";
      return Unsupported;
    }

    // Confidence is independent of writer options. IOUtil discovers a
    // writer through this method *before* it has a chance to apply the
    // caller's chosen options on the selected writer (writer selection
    // happens first, options are set afterwards), so reading options
    // here yields the defaults and not the caller's intent. The actual
    // strict-mode / synthetic-mode policy is enforced inside Write,
    // which has access to the configured options and surfaces a
    // mitk::Exception when an incomplete seg meets strict mode.
    return Supported;
  }

  void DICOMSegmentationIO::Write()
  {
    ValidateOutputLocation();

    mitk::LocaleSwitch localeSwitch("C");
    LocalFile localFile(this);
    const std::string path = localFile.GetFileName();

    // AbstractFileWriter::GetInput() returns const BaseData*. Synthesis
    // mutates the seg's property list (Complete fills missing identity
    // tags in place) so the writer needs a non-const handle on the
    // caller's seg. const_cast is the standard workaround for this
    // framework limitation; the mutation is documented and confined to
    // Complete(). Reviewed exception to the CLAUDE.md const_cast rule.
    const auto *constInput = dynamic_cast<const MultiLabelSegmentation *>(this->GetInput());
    if (constInput == nullptr)
      mitkThrow() << "Cannot write non-MultiLabelSegmentation data via DICOM SEG.";
    auto *input = const_cast<MultiLabelSegmentation *>(constInput);

    const auto options = this->AbstractFileWriter::GetOptions();
    const std::string synthesisMode = ResolveEnumOption(options, OPTION_SYNTHESIS_MODE,
                                                        OPTION_SYNTHESIS_MODE_STRICT);
    const std::string encoding = ResolveEnumOption(options, OPTION_ENCODING,
                                                   OPTION_ENCODING_LABELMAP);

    const bool isSynthetic = (synthesisMode == OPTION_SYNTHESIS_MODE_SYNTHETIC);

    // Synthesis runs once before the per-group loop. The alternative
    // (per-group) would invite divergence across groups when the
    // synthesised top-level identity tags are minted with random UIDs.
    if (isSynthetic)
    {
      DICOMSegmentationPropertyHelper::CompletionOptions completionOptions;
      completionOptions.synthesizeMissingIdentity = true;
      completionOptions.deriveGeometryFromSegmentation = true;
      DICOMSegmentationPropertyHelper::Complete(input, completionOptions);
    }

    const auto missing = DICOMSegmentationPropertyHelper::Validate(input);
    if (!missing.empty())
    {
      std::ostringstream msg;
      msg << "DICOM SEG write blocked by strict mode: the segmentation is "
          << "missing " << missing.size() << " contract item(s) required to "
          << "produce a valid SEG. Set the writer option \""
          << OPTION_SYNTHESIS_MODE << "\" to \"" << OPTION_SYNTHESIS_MODE_SYNTHETIC
          << "\" to fill these automatically, or populate them explicitly. "
          << "Missing items:";
      for (const auto &m : missing)
        msg << "\n  - " << FormatMissingItem(m);
      mitkThrow() << msg.str();
    }

    const bool wantLabelmap = (encoding == OPTION_ENCODING_LABELMAP);

    for (unsigned int layer = 0; layer < input->GetNumberOfGroups(); ++layer)
    {
      std::vector<itkInternalImageType::ConstPointer> segmentations;

      try
      {
        auto mitkLayerImage = input->GetGroupImage(layer);

        ImageToItk<itkInputImageType>::Pointer imageToItkFilter = ImageToItk<itkInputImageType>::New();
        imageToItkFilter->SetInput(mitkLayerImage);
        typedef itk::CastImageFilter<itkInputImageType, itkInternalImageType> castItkImageFilterType;
        castItkImageFilterType::Pointer castFilter = castItkImageFilterType::New();
        castFilter->SetInput(imageToItkFilter->GetOutput());
        castFilter->Update();

        itkInternalImageType::Pointer itkLabelImage = castFilter->GetOutput();
        itkLabelImage->DisconnectPipeline();

        auto labelSet = input->GetConstLabelsByValue(input->GetLabelValuesByGroup(layer));

        for (const auto &label : labelSet)
        {
          itk::ThresholdImageFilter<itkInternalImageType>::Pointer thresholdFilter =
            itk::ThresholdImageFilter<itkInternalImageType>::New();
          thresholdFilter->SetInput(itkLabelImage);
          thresholdFilter->ThresholdOutside(label->GetValue(), label->GetValue());
          thresholdFilter->SetOutsideValue(0);
          thresholdFilter->Update();
          itkInternalImageType::Pointer segmentImage = thresholdFilter->GetOutput();
          segmentImage->DisconnectPipeline();

          segmentations.push_back(segmentImage);
        }
      }
      catch (const itk::ExceptionObject &e)
      {
        mitkThrow() << "ITK error preparing segment images for DICOM SEG group "
                    << layer << ": " << e.GetDescription();
      }

      dcmqi::JSONSegmentationMetaInformationHandler handler;
      this->BuildMetaInfoHandler(input, static_cast<int>(layer), handler);

      auto sourceItems = BuildSourceItemsFromProperties(input, layer);
      if (sourceItems.empty())
      {
        if (isSynthetic)
        {
          // Synthetic mode safety net: no real source-image relation is
          // attached, so emit a minimal phantom-source set so dcmqi's
          // dcmDatasets[0] anchor is well-formed. The resulting SEG's
          // ReferencedSeriesSequence points at a synthesised series in
          // the seg's own FoR; downstream tooling that recognises
          // MITK-minted UIDs can treat them as such.
          sourceItems = BuildSyntheticSourceItems(input, layer);
        }
        if (sourceItems.empty())
        {
          mitkThrow() << "DICOM SEG write requires at least one source-image"
                      << " reference for group " << layer << ", but the"
                      << " segmentation carries no SegSourceImageRelationRule"
                      << " connection with usable per-slice SOPInstance UIDs."
                      << " Attach a real source via"
                      << " LabelSetImageHelper::SetupDerivedSegmentation"
                      << " (or SegSourceImageRelationRule::Connect directly),"
                      << " or set the writer option \""
                      << OPTION_SYNTHESIS_MODE << "\" to \""
                      << OPTION_SYNTHESIS_MODE_SYNTHETIC
                      << "\" to synthesise a placeholder source series.";
        }
      }
      // dcmqi's API takes a raw pointer vector; unique_ptr ownership stays
      // on the stack in sourceItems and outlives the converter call.
      std::vector<DcmItem *> rawSourceItems;
      rawSourceItems.reserve(sourceItems.size());
      for (const auto &item : sourceItems)
        rawSourceItems.push_back(item.get());

      const bool useLabelIDAsSegmentNumber = wantLabelmap && LabelsAreMonotonicOneToN(input, layer);

      // Per-layer output path, also used by the log line so the message
      // names the file actually being written.
      std::string filePath = path.substr(0, path.find_last_of("."));
      if (input->GetNumberOfGroups() != 1)
        filePath = filePath + std::to_string(layer) + ".dcm";
      else
        filePath = filePath + ".dcm";

      MITK_INFO << "Writing DICOM SEG group " << layer << " to " << filePath;
      try
      {
        // doDicomValueChecks left at the dcmqi default (true): synthesis is
        // required to produce VR-valid values, enforced by a dedicated unit
        // test. There is no MITK-side dial for this.
        auto converter = std::make_unique<dcmqi::Itk2DicomConverter>();
        std::unique_ptr<DcmDataset> result(converter->itkimage2dcmSegmentation(
          rawSourceItems,
          segmentations,
          handler,
          /*skipEmptySlices=*/true,
          /*useLabelIDAsSegmentNumber=*/useLabelIDAsSegmentNumber,
          /*referencesGeometryCheck=*/true,
          /*doDicomValueChecks=*/true,
          /*outputLabelMap=*/wantLabelmap));

        if (result == nullptr)
          mitkThrow() << "dcmqi failed to convert the segmentation to DICOM SEG for group " << layer << ".";

        // Within one MITK group labels are non-overlapping by construction,
        // so the SEG-level SegmentsOverlap tag is "NO" regardless of
        // encoding (labelmap also forbids overlap by Sup 243).
        if (result->putAndInsertString(DCM_SegmentsOverlap, "NO").bad())
          MITK_DEBUG << "Unable to set SegmentsOverlap tag.";

        DcmFileFormat dcmFileFormat(result.get());

        const auto saveCond = dcmFileFormat.saveFile(filePath.c_str(), EXS_LittleEndianExplicit);
        if (saveCond.bad())
          mitkThrow() << "Failed to write DICOM SEG group " << layer
                      << " to " << filePath << ": " << saveCond.text();
      }
      // mitk::Exception derives from std::exception. Catching it first and
      // rethrowing preserves the exception type for callers that route on
      // mitk::Exception specifically; only genuine non-MITK exceptions
      // (dcmqi / DCMTK / std) get wrapped into a uniform mitkThrow below.
      catch (const mitk::Exception &)
      {
        throw;
      }
      catch (const std::exception &e)
      {
        mitkThrow() << "Error while writing DICOM SEG group " << layer
                    << ": " << e.what();
      }
    }
  }

  IFileIO::ConfidenceLevel DICOMSegmentationIO::GetReaderConfidenceLevel() const
  {
    if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
      return Unsupported;

    const std::string fileName = this->GetLocalFileName();

    DcmFileFormat dcmFileFormat;
    OFCondition status = dcmFileFormat.loadFile(fileName.c_str());

    if (status.bad())
      return Unsupported;

    OFString modality;
    if (dcmFileFormat.getDataset()->findAndGetOFString(DCM_Modality, modality).good())
    {
      if (modality.compare("SEG") == 0)
        return Supported;
      else
        return Unsupported;
    }
    return Unsupported;
  }

  std::vector<BaseData::Pointer> DICOMSegmentationIO::DoRead()
  {
    mitk::LocaleSwitch localeSwitch("C");

    MultiLabelSegmentation::Pointer labelSetImage;
    std::vector<BaseData::Pointer> result;

    const std::string path = this->GetLocalFileName();

    MITK_INFO << "loading " << path << std::endl;

    if (path.empty())
      mitkThrow() << "Empty filename in mitk::ItkImageIO ";

    try
    {
      // Get the dcm data set from file path
      DcmFileFormat dcmFileFormat;
      OFCondition status = dcmFileFormat.loadFile(path.c_str());
      if (status.bad())
        mitkThrow() << "Can't read the input file!";

      DcmDataset *dataSet = dcmFileFormat.getDataset();
      if (dataSet == nullptr)
        mitkThrow() << "Can't read data from input file!";

      //=============================== dcmqi part ====================================
      // getConverter() is a SOP-Class-specific factory: UID_SegmentationStorage
      // returns the binary converter, UID_LabelMapSegmentationStorage (Sup 243)
      // returns the labelmap converter. The two paths build the
      // MultiLabelSegmentation differently (per-segment vs single labelmap
      // image), so dispatch happens once here and the helpers diverge from
      // there.
      std::unique_ptr<dcmqi::Dicom2ItkConverterBase> converter(
        dcmqi::Dicom2ItkConverter::getConverter(dataSet));
      if (converter == nullptr)
        mitkThrow() << "Unsupported DICOM SEG SOP Class; cannot read.";

      std::string metaInfoString;
      auto convertCondition = converter->dcmSegmentation2itkimage(dataSet, metaInfoString, false);
      if (convertCondition.bad())
        mitkThrow() << "dcmqi failed to convert DICOM SEG: "
                    << convertCondition.text();

      dcmqi::JSONSegmentationMetaInformationHandler metaInfo(metaInfoString.c_str());
      metaInfo.read();

      MITK_INFO << "Input " << metaInfo.getJSONOutputAsString();
      //===============================================================================

      if (converter->isLabelmap())
      {
        labelSetImage = this->ReadLabelmapSegmentation(*converter, metaInfo);
      }
      else
      {
        labelSetImage = this->ReadBinarySegmentation(
          *converter, metaInfo, ShouldAssumeOverlappingSegments(*dataSet));
      }

      if (labelSetImage.IsNull())
        mitkThrow() << "No valid segments found in DICOM SEG file.";

      labelSetImage->SetAllLabelsVisible(true);

      if (labelSetImage->GetTotalNumberOfLabels() > 0)
      {
        labelSetImage->SetActiveLabel(labelSetImage->GetAllLabelValues().front());
      }

      // Add some general DICOM Segmentation properties
      mitk::IDICOMTagsOfInterest *toiSrv = DICOMIOHelper::GetTagsOfInterestService();
      if (toiSrv == nullptr)
        mitkThrow() << "DICOM tags-of-interest service is not available.";

      auto tagsOfInterest = toiSrv->GetTagsOfInterest();
      DICOMTagPathList tagsOfInterestList;
      for (const auto &tag : tagsOfInterest)
      {
        tagsOfInterestList.push_back(tag.first);
      }

      mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
      scanner->SetInputFiles({ GetInputLocation() });
      scanner->AddTagPaths(tagsOfInterestList);
      scanner->Scan();

      mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
      if (frames.empty())
      {
        MITK_ERROR << "Error reading the DICOM Seg file" << std::endl;
        return result;
      }

      auto findings = DICOMIOHelper::ExtractPathsOfInterest(tagsOfInterestList, frames);
      DICOMIOHelper::SetProperties(labelSetImage, findings);

      // Populate after SetProperties so the per-slice TemporoSpatialString
      // properties this writes (DICOM.0008.2112.[0].0008.1155 and friends)
      // cannot be flattened by the tag-of-interest scanner. Parse from the
      // already-loaded dataset rather than re-reading the file: dcmqi above
      // does not take ownership of dcmFileFormat's dataset and dcmFileFormat
      // outlives this block, so loadDataset avoids a second disk read.
      DcmSegmentation* segDocRaw = nullptr;
      OFCondition loadSegCond = DcmSegmentation::loadDataset(*dataSet, segDocRaw);
      std::unique_ptr<DcmSegmentation> segDoc(segDocRaw);
      if (loadSegCond.bad() || segDoc == nullptr)
        mitkThrow() << "Failed to parse DICOM SEG via DcmSegmentation::loadDataset: "
                    << loadSegCond.text();

      const auto frameRefs = CollectPerFrameSourceRefs(*segDoc);
      const auto seriesToInstances = CollectSourceInstancesBySeries(*dataSet);
      const auto sourceSeriesGroups = GroupPerFrameRefsBySeries(frameRefs, seriesToInstances);
      PopulateSourceImageRelations(*labelSetImage, sourceSeriesGroups);
    }
    catch (const std::exception &e)
    {
      MITK_ERROR << "An error occurred while reading the DICOM Seg file: " << e.what();
      return result;
    }
    catch (...)
    {
      MITK_ERROR << "An error occurred in dcmqi while reading the DICOM Seg file";
      return result;
    }

    result.push_back(labelSetImage.GetPointer());
    return result;
  }

  mitk::MultiLabelSegmentation::Pointer DICOMSegmentationIO::ReadBinarySegmentation(
    dcmqi::Dicom2ItkConverterBase &converter,
    dcmqi::JSONSegmentationMetaInformationHandler &metaInfo,
    bool assumeOverlappingSegments)
  {
    std::vector<itkInternalImageType::Pointer> segItkImages;
    auto image = converter.begin16Bit();
    while (image.IsNotNull())
    {
      segItkImages.emplace_back(image);
      image = converter.next16Bit();
    }

    if (segItkImages.empty())
      mitkThrow() << "DICOM SEG converted successfully but yielded no "
                  << "segment images; cannot construct a MultiLabelSegmentation.";

    MultiLabelSegmentation::Pointer labelSetImage;

    // Get the label information from segment attributes for each itk image
    vector<map<unsigned, dcmqi::SegmentAttributes *>>::const_iterator segmentIter =
      metaInfo.segmentsAttributesMappingList.begin();

    // For each itk image add a layer to the MultiLabelSegmentation output
    for (auto &segItkImage : segItkImages)
    {
      // Get the labeled image and cast it to mitkImage
      typedef itk::CastImageFilter<itkInternalImageType, itkInputImageType> castItkImageFilterType;
      castItkImageFilterType::Pointer castFilter = castItkImageFilterType::New();
      castFilter->SetInput(segItkImage);
      castFilter->Update();

      Image::Pointer segmentImage;
      CastToMitkImage(castFilter->GetOutput(), segmentImage);

      // Get pixel value of the label
      itkInternalImageType::ValueType segValue = 1;
      typedef itk::ImageRegionIterator<const itkInternalImageType> IteratorType;
      // Iterate over the image to find the pixel value of the label
      IteratorType iter(segItkImage, segItkImage->GetLargestPossibleRegion());
      iter.GoToBegin();
      while (!iter.IsAtEnd())
      {
        itkInputImageType::PixelType value = iter.Get();
        if (value != MultiLabelSegmentation::UNLABELED_VALUE)
        {
          segValue = value;
          break;
        }
        ++iter;
      }
      // Get Segment information map
      if (segmentIter == metaInfo.segmentsAttributesMappingList.end())
        mitkThrow() << "Segment metadata list has fewer entries than segment images.";

      const auto &segmentMap = (*segmentIter);
      if (segmentMap.empty())
        mitkThrow() << "Segment metadata entry is empty for segment image.";

      dcmqi::SegmentAttributes *segmentAttribute = segmentMap.begin()->second;
      if (segmentAttribute == nullptr)
        mitkThrow() << "Segment attributes are null for segment image.";

      const OFString labelName = DeriveLabelName(*segmentAttribute);

      float tmp[3] = { 0.0, 0.0, 0.0 };
      if (segmentAttribute->getRecommendedDisplayRGBValue() != nullptr)
      {
        tmp[0] = segmentAttribute->getRecommendedDisplayRGBValue()[0] / 255.0;
        tmp[1] = segmentAttribute->getRecommendedDisplayRGBValue()[1] / 255.0;
        tmp[2] = segmentAttribute->getRecommendedDisplayRGBValue()[2] / 255.0;
      }

      Label::Pointer newLabel = nullptr;
      // If labelSetImage do not exists (first image)
      if (labelSetImage.IsNull())
      {
        // Initialize the labelSetImage with the read image
        labelSetImage = MultiLabelSegmentation::New();
        labelSetImage->InitializeByLabeledImage(segmentImage);
        // Check if the segment image contained labeled pixels. At this point it either contains no (when no labeled pixels where in the image)
        // or one label (as DCMSeg segments only represent one labels). So either generate a new label or used the only existing one.
        newLabel = labelSetImage->GetTotalNumberOfLabels() > 0 ? labelSetImage->GetLabels().front() : Label::New();
        newLabel->SetName(labelName.c_str());
        newLabel->SetColor(Color(tmp));
        newLabel->SetValue(segValue);
      }
      else
      {
        MultiLabelSegmentation::GroupIndexType groupID = 0;
        if (assumeOverlappingSegments)
        {
          // Add a new group because we have to expect every label to be overlapping
          // the label content is directly transferred here.
          groupID = labelSetImage->AddGroup(segmentImage);
        }

        // Add the new label
        newLabel = Label::New();
        newLabel->SetName(labelName.c_str());
        newLabel->SetColor(Color(tmp));
        newLabel->SetValue(segValue);
        labelSetImage->AddLabel(newLabel, groupID, true, true);

        if (!assumeOverlappingSegments)
        {
          //if we know the labels are non overlapping we can put everything in one image
          //the label content has to be transferred, as no new group was added.
          mitk::TransferLabelContent(segmentImage, labelSetImage->GetGroupImage(groupID),
            labelSetImage->GetConstLabelsByValue(labelSetImage->GetLabelValuesByGroup(groupID)),
            mitk::MultiLabelSegmentation::UNLABELED_VALUE, mitk::MultiLabelSegmentation::UNLABELED_VALUE, false, {{segValue,newLabel->GetValue()}});
        }
      }

      // Add some more label properties
      this->SetLabelProperties(newLabel, segmentAttribute);
      ++segmentIter;
    }

    return labelSetImage;
  }

  mitk::MultiLabelSegmentation::Pointer DICOMSegmentationIO::ReadLabelmapSegmentation(
    dcmqi::Dicom2ItkConverterBase &converter,
    dcmqi::JSONSegmentationMetaInformationHandler &metaInfo)
  {
    const Uint8 bytesPerPixel = converter.bytesPerPixel();
    if (bytesPerPixel != 1 && bytesPerPixel != 2)
      mitkThrow() << "Unsupported labelmap pixel size: bytesPerPixel="
                  << static_cast<unsigned>(bytesPerPixel)
                  << " (expected 1 or 2 per dcmqi's Sup 243 converter).";

    // dcmqi's labelmap converter emits exactly one image carrying every
    // segment number as its pixel values. The multi-image guard surfaces
    // a future dcmqi change explicitly instead of silently dropping all
    // but the first image.
    Image::Pointer mitkSegImage;
    int imageCount = 0;
    if (bytesPerPixel == 2)
    {
      auto image = converter.begin16Bit();
      while (image.IsNotNull())
      {
        if (++imageCount > 1)
          mitkThrow() << "Labelmap DICOM SEG converter yielded more than one "
                      << "image; Sup 243 expects exactly one per SEG.";
        typedef itk::CastImageFilter<itkInternalImageType, itkInputImageType> CastFilter;
        auto castFilter = CastFilter::New();
        castFilter->SetInput(image);
        castFilter->Update();
        CastToMitkImage(castFilter->GetOutput(), mitkSegImage);
        image = converter.next16Bit();
      }
    }
    else // bytesPerPixel == 1
    {
      typedef itk::Image<unsigned char, 3> Uint8ImageType;
      auto image = converter.begin8Bit();
      while (image.IsNotNull())
      {
        if (++imageCount > 1)
          mitkThrow() << "Labelmap DICOM SEG converter yielded more than one "
                      << "image; Sup 243 expects exactly one per SEG.";
        typedef itk::CastImageFilter<Uint8ImageType, itkInputImageType> CastFilter;
        auto castFilter = CastFilter::New();
        castFilter->SetInput(image);
        castFilter->Update();
        CastToMitkImage(castFilter->GetOutput(), mitkSegImage);
        image = converter.next8Bit();
      }
    }

    if (imageCount == 0)
      mitkThrow() << "Labelmap DICOM SEG converted successfully but yielded "
                  << "no image; cannot construct a MultiLabelSegmentation.";

    if (metaInfo.segmentsAttributesMappingList.empty())
      mitkThrow() << "Labelmap DICOM SEG yielded no segment attribute entries.";

    // The SEG's Segment Sequence is authoritative for what labels exist;
    // pixel values are content. Sup 243 may declare a Background segment
    // (number 0); MITK treats pixel value 0 as UNLABELED, so background
    // entries are dropped here to avoid colliding with foreground handling.
    const auto &segmentMap = metaInfo.segmentsAttributesMappingList.front();
    std::map<MultiLabelSegmentation::LabelValueType, dcmqi::SegmentAttributes *> attributesByLabelValue;
    for (const auto &segmentEntry : segmentMap)
    {
      dcmqi::SegmentAttributes *segmentAttribute = segmentEntry.second;
      if (segmentAttribute == nullptr)
        continue;
      const auto labelValue =
        static_cast<MultiLabelSegmentation::LabelValueType>(segmentAttribute->getLabelID());
      if (labelValue == MultiLabelSegmentation::UNLABELED_VALUE)
        continue;
      attributesByLabelValue.emplace(labelValue, segmentAttribute);
    }

    // The labelmap image's pixel values are the segment numbers, so
    // InitializeByLabeledImage builds one MITK group with one auto-created
    // Label per distinct value. The labels are then re-driven from metadata
    // below; auto-creation is only used to bind label values to pixel-grid
    // content.
    auto labelSetImage = MultiLabelSegmentation::New();
    labelSetImage->InitializeByLabeledImage(mitkSegImage);

    // Sup 243 requires every non-zero pixel value to be described by a
    // Segment Sequence entry. Mismatch in either direction is a
    // non-conformant SEG and throws, matching the binary branch's
    // hard-failure contract (no silent drops).
    for (const auto labelValue : labelSetImage->GetAllLabelValues())
    {
      if (labelValue == MultiLabelSegmentation::UNLABELED_VALUE)
        continue;
      if (attributesByLabelValue.find(labelValue) == attributesByLabelValue.end())
        mitkThrow() << "Labelmap DICOM SEG contains pixel value "
                    << static_cast<unsigned>(labelValue)
                    << " but no matching Segment Sequence entry; SEG is non-conformant.";
    }

    // Drive label naming, colour and DICOM property metadata from
    // segmentsAttributesMappingList. SetLabelProperties also stamps the
    // empty tracking-ID/UID sentinel that suppresses MITK's auto-UID
    // generation, so running it on every metadata-described label keeps
    // round-trip integrity.
    for (const auto &[labelValue, segmentAttribute] : attributesByLabelValue)
    {
      Label *label = labelSetImage->GetLabel(labelValue);
      if (label == nullptr)
        mitkThrow() << "Labelmap DICOM SEG metadata declares segment "
                    << static_cast<unsigned>(labelValue)
                    << " but no pixel with that value is present; SEG is non-conformant.";

      const OFString labelName = DeriveLabelName(*segmentAttribute);
      label->SetName(labelName.c_str());

      if (segmentAttribute->getRecommendedDisplayRGBValue() != nullptr)
      {
        const float rgb[3] = {
          segmentAttribute->getRecommendedDisplayRGBValue()[0] / 255.0f,
          segmentAttribute->getRecommendedDisplayRGBValue()[1] / 255.0f,
          segmentAttribute->getRecommendedDisplayRGBValue()[2] / 255.0f
        };
        label->SetColor(Color(rgb));
      }

      this->SetLabelProperties(label, segmentAttribute);
    }

    return labelSetImage;
  }

  void mitk::DICOMSegmentationIO::BuildMetaInfoHandler(const MultiLabelSegmentation *input,
                                                       int layer,
                                                       dcmqi::JSONSegmentationMetaInformationHandler &handler) const
  {
    if (input == nullptr)
      mitkThrow() << "BuildMetaInfoHandler: input must not be nullptr.";

    const mitk::MultiLabelSegmentation *image = input;

    // 1. Metadata attributes that will be listed in the resulting DICOM SEG object
    std::string contentCreatorName;
    if (!image->GetPropertyList()->GetStringProperty(GeneratePropertyNameForDICOMTag(0x0070, 0x0084).c_str(),
      contentCreatorName))
      contentCreatorName = "MITK";
    handler.setContentCreatorName(contentCreatorName);

    std::string clinicalTrailSeriesId;
    if (!image->GetPropertyList()->GetStringProperty(GeneratePropertyNameForDICOMTag(0x0012, 0x0071).c_str(),
      clinicalTrailSeriesId))
      clinicalTrailSeriesId = "Session 1";
    handler.setClinicalTrialSeriesID(clinicalTrailSeriesId);

    std::string clinicalTrialTimePointID;
    if (!image->GetPropertyList()->GetStringProperty(GeneratePropertyNameForDICOMTag(0x0012, 0x0050).c_str(),
      clinicalTrialTimePointID))
      clinicalTrialTimePointID = "0";
    handler.setClinicalTrialTimePointID(clinicalTrialTimePointID);

    std::string clinicalTrialCoordinatingCenterName = "";
    if (!image->GetPropertyList()->GetStringProperty(GeneratePropertyNameForDICOMTag(0x0012, 0x0060).c_str(),
      clinicalTrialCoordinatingCenterName))
      clinicalTrialCoordinatingCenterName = "Unknown";
    handler.setClinicalTrialCoordinatingCenterName(clinicalTrialCoordinatingCenterName);

    std::string seriesDescription;
    if (!image->GetPropertyList()->GetStringProperty("name", seriesDescription))
      seriesDescription = "MITK Segmentation";
    handler.setSeriesDescription(seriesDescription);

    handler.setSeriesNumber("0" + std::to_string(layer));
    handler.setInstanceNumber("1");
    handler.setBodyPartExamined("");

    auto labelSet = image->GetConstLabelsByValue(image->GetLabelValuesByGroup(layer));

    unsigned int segmentNumber = 0;

    for (const auto& label : labelSet)
    {
      ++segmentNumber;
      if (label != nullptr)
      {
        auto segmentAttribute = handler.createOrGetSegment(segmentNumber, label->GetValue());

        if (segmentAttribute != nullptr)
        {
          segmentAttribute->setSegmentLabel(label->GetName());
          segmentAttribute->setSegmentDescription(label->GetDescription());
          std::string algorithmType = label->GetAlgorithmTypeStr();
          if (algorithmType.empty())
            algorithmType = "MANUAL"; //DICOM always needs a type. If undefined we default to "MANUAL"
          segmentAttribute->setSegmentAlgorithmType(algorithmType);
          segmentAttribute->setSegmentAlgorithmName(label->GetAlgorithmName());

          if (label->GetAnatomicRegionCount()>0)
          { //Anatomic region
            auto anatomicRegion = label->GetAnatomicRegion(0); //currently DCMQI only supports one anatomic region code

            if (!anatomicRegion.GetValue().empty() && !anatomicRegion.GetScheme().empty() && !anatomicRegion.GetMeaning().empty())
            {
              segmentAttribute->setAnatomicRegionSequence(anatomicRegion.GetValue(),
                anatomicRegion.GetScheme(), anatomicRegion.GetMeaning());
              handler.setBodyPartExamined(anatomicRegion.GetMeaning());
            }

            if (anatomicRegion.GetModifierCount() > 0)
            {
              //Anatomic Region Modifier (DCMQI only supports one modifier)
              auto arMod = anatomicRegion.GetModifier(0);

              if (!arMod.GetValue().empty() && !arMod.GetScheme().empty() && !arMod.GetMeaning().empty())
              {
                segmentAttribute->setAnatomicRegionModifierSequence(arMod.GetValue(),
                  arMod.GetScheme(), arMod.GetMeaning());
              }
            }

          }

          { //Segment category
            auto segCat = label->GetSegmentedPropertyCategory();

            if (segCat.has_value() && !segCat->GetValue().empty() && !segCat->GetScheme().empty() && !segCat->GetMeaning().empty())
            {
              segmentAttribute->setSegmentedPropertyCategoryCodeSequence(segCat->GetValue(),
                segCat->GetScheme(), segCat->GetMeaning());
              handler.setBodyPartExamined(segCat->GetMeaning());
            }
            else
            {
              // SEG IOD requires (0062,0003) (Type 1). Emit an honest "unknown"
              // stand-in rather than blocking the write or claiming specific
              // semantics. BodyPartExamined is deliberately not stamped here;
              // an "Unknown" body part is worse than the field staying empty.
              segmentAttribute->setSegmentedPropertyCategoryCodeSequence(
                "49755003", "SCT", "Morphologically altered structure");
            }
          }

          {
            auto segType = label->GetSegmentedPropertyType();

            if (segType.has_value() && !segType->GetValue().empty() && !segType->GetScheme().empty() && !segType->GetMeaning().empty())
            {
              segmentAttribute->setSegmentedPropertyTypeCodeSequence(segType->GetValue(),
                segType->GetScheme(), segType->GetMeaning());
            }
            else
            {
              // SEG IOD requires (0062,000F) (Type 1). Pair the same SCT base
              // as Category with an "Unknown" modifier so the output reads
              // "morphological alteration of unspecified kind" rather than
              // picking a specific morphology.
              segmentAttribute->setSegmentedPropertyTypeCodeSequence(
                "49755003", "SCT", "Morphologically altered structure");
              segmentAttribute->setSegmentedPropertyTypeModifierCodeSequence(
                "261665006", "SCT", "Unknown (qualifier value)");
            }

            if (segType.has_value() && segType->GetModifierCount() > 0)
            {
              //Segment Type Modifier (DCMQI only supports one modifier
              auto segTypeMod = segType->GetModifier(0);

              if (!segTypeMod.GetValue().empty() && !segTypeMod.GetScheme().empty() && !segTypeMod.GetMeaning().empty())
              {
                segmentAttribute->setSegmentedPropertyTypeModifierCodeSequence(segTypeMod.GetValue(),
                  segTypeMod.GetScheme(), segTypeMod.GetMeaning());
              }
            }
          }

          Color color = label->GetColor();
          segmentAttribute->setRecommendedDisplayRGBValue(color[0] * 255, color[1] * 255, color[2] * 255);

          if (!label->GetTrackingID().empty())
            segmentAttribute->setTrackingIdentifier(label->GetTrackingID());
          if (!label->GetTrackingUID().empty())
            segmentAttribute->setTrackingUniqueIdentifier(label->GetTrackingUID());
        }
      }
    }
  }

  void mitk::DICOMSegmentationIO::SetLabelProperties(mitk::Label *label, dcmqi::SegmentAttributes *segmentAttribute)
  {
    // Segment Algorithm Type: Type of algorithm used to generate the segment.
    label->SetAlgorithmTypeStr(segmentAttribute->getSegmentAlgorithmType());
    label->SetAlgorithmName(segmentAttribute->getSegmentAlgorithmName());

    // Add Segmented Property Category Code Sequence tags
    auto categoryCodeSequence = segmentAttribute->getSegmentedPropertyCategoryCodeSequence();
    if (categoryCodeSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      categoryCodeSequence->getCodeValue(codeValue);
      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      categoryCodeSequence->getCodingSchemeDesignator(codeScheme);
      OFString codeMeaning; // (0008,0104) Code Meaning
      categoryCodeSequence->getCodeMeaning(codeMeaning);

      label->SetSegmentedPropertyCategory(DICOMCodeSequence(codeValue, codeScheme, codeMeaning));
    }

    // Add Segmented Property Type Code Sequence tags
    auto typeCodeSequence = segmentAttribute->getSegmentedPropertyTypeCodeSequence();
    if (typeCodeSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      typeCodeSequence->getCodeValue(codeValue);
      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      typeCodeSequence->getCodingSchemeDesignator(codeScheme);
      OFString codeMeaning; // (0008,0104) Code Meaning
      typeCodeSequence->getCodeMeaning(codeMeaning);

      DICOMCodeSequenceWithModifiers code = DICOMCodeSequenceWithModifiers(codeValue, codeScheme, codeMeaning);

      // Add Segmented Property Type Modifier Code Sequence tags
      auto modifierCodeSequence = segmentAttribute->getSegmentedPropertyTypeModifierCodeSequence();
      if (modifierCodeSequence != nullptr)
      {
        OFString modValue; // (0008,0100) Code Value
        modifierCodeSequence->getCodeValue(modValue);
        OFString modScheme; // (0008,0102) Coding Scheme Designator
        modifierCodeSequence->getCodingSchemeDesignator(modScheme);
        OFString modMeaning; // (0008,0104) Code Meaning
        modifierCodeSequence->getCodeMeaning(modMeaning);
        code.AddModifier(DICOMCodeSequence(modValue, modScheme, modMeaning));
      }

      label->SetSegmentedPropertyType(code);
    }

    // Add Anatomic RegionSequence tags
    auto atomicRegionSequence = segmentAttribute->getAnatomicRegionSequence();
    if (atomicRegionSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      atomicRegionSequence->getCodeValue(codeValue);
      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      atomicRegionSequence->getCodingSchemeDesignator(codeScheme);
      OFString codeMeaning; // (0008,0104) Code Meaning
      atomicRegionSequence->getCodeMeaning(codeMeaning);

      DICOMCodeSequenceWithModifiers code = DICOMCodeSequenceWithModifiers(codeValue, codeScheme, codeMeaning);
      // Add Anatomic Region Modifier Code Sequence tags
      auto modifierCodeSequence = segmentAttribute->getAnatomicRegionModifierSequence();
      if (modifierCodeSequence != nullptr)
      {
        OFString modValue; // (0008,0100) Code Value
        modifierCodeSequence->getCodeValue(modValue);
        OFString modScheme; // (0008,0102) Coding Scheme Designator
        modifierCodeSequence->getCodingSchemeDesignator(modScheme);
        OFString modMeaning; // (0008,0104) Code Meaning
        modifierCodeSequence->getCodeMeaning(modMeaning);
        code.AddModifier(DICOMCodeSequence(modValue, modScheme, modMeaning));
      }

      label->SetAnatomicRegion(code);
    }

    // Only set the tracking property when the source DICOM carried a
    // non-empty value, so HasTrackingID/UID reflects the source truthfully.
    const auto readTrackingID = segmentAttribute->getTrackingIdentifier();
    if (!readTrackingID.empty())
      label->SetTrackingID(readTrackingID);
    const auto readTrackingUID = segmentAttribute->getTrackingUniqueIdentifier();
    if (!readTrackingUID.empty())
      label->SetTrackingUID(readTrackingUID);
  }

  DICOMSegmentationIO *DICOMSegmentationIO::IOClone() const { return new DICOMSegmentationIO(*this); }
} // namespace
