/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegIOTestCapabilities_h
#define mitkSegIOTestCapabilities_h

#include <mitkSegIOTestComparator.h>

#include <mitkImageWriteAccessor.h>
#include <mitkStringProperty.h>

#include <dcmqi/ColorUtilities.h>

#include <cmath>
#include <map>
#include <string>
#include <vector>

/**
 * The capability table of the segmentation file formats: per format, what
 * a round trip loses (as the edit that reproduces the loss on the expected
 * side) and how exactly it reproduces what it keeps. A loss is only listed
 * when the format cannot carry the attribute, the upstream library cannot
 * carry it (with an upstream ticket), or the owner decided so; anything else
 * MITK drops is a defect.
 */
namespace mitk
{
  namespace test
  {
    namespace detail
    {
      /** Keys of DICOM tags and of relations, which have dedicated persistence infos. */
      inline bool HasDedicatedPersistence(const std::string& key)
      {
        const auto relationRoot = PropertyKeyPathToPropertyName(PropertyRelationRuleBase::GetRootKeyPath()) + ".";
        return StartsWith(key, "DICOM.") || StartsWith(key, relationRoot);
      }
    }

    /** Native Multi-Label NRRD (MultiLabelSegmentationIO). */
    inline FormatCapabilities NativeNrrdCapabilities()
    {
      FormatCapabilities capabilities;
      capabilities.formatName = "native NRRD";

      // The label colours travel as JSON floats, which round trip exactly.
      capabilities.options.colorTolerance = 0.f;
      // NRRD stores origin and space directions with 17 significant digits; what remains is
      // the floating point split of the index-to-world matrix into spacing and direction.
      capabilities.options.geometryTolerance = 1e-9;
      // ArbitraryTimeGeometry time points are written with the stream default of 6 significant
      // digits (ConvertTimePointListToMetaDataObject); for time points below 100 that bounds the
      // error by 5e-5.
      capabilities.options.timeTolerance = 1e-4;

      capabilities.losses.push_back({
        "custom segmentation properties come back as StringProperty: the default persistence info"
        " serialises the value as string and deserialises it as StringProperty (owner decision)",
        [](MultiLabelSegmentation& segmentation) {
          auto properties = segmentation.GetPropertyList();
          for (const auto& key : properties->GetPropertyKeys())
          {
            if (detail::HasDedicatedPersistence(key))
              continue;
            const auto value = properties->GetProperty(key)->GetValueAsString();
            properties->RemoveProperty(key);
            properties->SetProperty(key, StringProperty::New(value));
          }
        } });

      return capabilities;
    }

    /** Segmentation stack (MultiLabelSegmentationStackReader, NIfTI stack writer); the same for every save strategy. */
    inline FormatCapabilities StackCapabilities()
    {
      FormatCapabilities capabilities;
      capabilities.formatName = "stack";

      // The label colours travel as JSON floats, which round trip exactly.
      capabilities.options.colorTolerance = 0.f;
      // NIfTI stores origin, spacing and orientation as float32, which rounds to 2^-24 relative.
      // At the largest coordinate of the fixtures, 30.25 mm, that bounds the error by 1.8e-6.
      capabilities.options.geometryTolerance = 2e-6;
      // The writer refuses dynamic segmentations, and the reload rebuilds the same single
      // time step, so its bounds come back exactly.
      capabilities.options.timeTolerance = 0.0;

      capabilities.losses.push_back({
        "the unlabeled lock: the stack format has no field for it, and the reader leaves it unlocked",
        [](MultiLabelSegmentation& segmentation) { segmentation.SetUnlabeledLabelLock(false); } });

      return capabilities;
    }

    /** One documented loss on a single label: why it happens, and the edit that reproduces it. */
    struct DocumentedLabelLoss
    {
      std::string description;
      std::function<void(Label&)> apply;
    };

    namespace detail
    {
      /** The indices of a label's codes under baseName, e.g. its anatomic regions; they can be sparse. */
      inline std::vector<PropertyKeyPath::ItemSelectionIndex> FindLabelCodeIndices(const Label& label,
                                                                                   const std::string& baseName)
      {
        PropertyKeyPath path;
        path.AddAnySelection(baseName);
        path.AddElement(LabelPropertyConstants::GetValuePropertySubName());

        std::vector<PropertyKeyPath::ItemSelectionIndex> indices;
        for (const auto& entry : FindIndexedPropertyNames(&label, path))
          indices.push_back(entry.first);
        return indices;
      }

      /** Keys of the label attributes DICOM SEG carries, and the prefixes of their codes. */
      inline bool IsDICOMSegLabelAttribute(const std::string& key)
      {
        static const std::set<std::string> keys = { "name", "color", "opacity", "visible", "locked", "description",
                                                    "tracking_id", "tracking_uid", "algorithm_type", "algorithm_name" };
        static const std::vector<std::string> codePrefixes = {
          LabelPropertyConstants::GetSegmentedPropertyCategoryPropertyBaseName(),
          LabelPropertyConstants::GetSegmentedPropertyTypePropertyBaseName(),
          LabelPropertyConstants::GetAnatomicRegionPropertyBaseName(),
          LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName() };
        return keys.count(key) > 0 || StartsWithAny(key, codePrefixes);
      }

      /** The colour as it comes back from DICOM SEG: rounded to 8 bit sRGB, then to integer CIELab and back with dcmqi. */
      inline Color DICOMSegColorRoundTrip(const Color& color)
      {
        int rgb[3];
        for (unsigned int channel = 0; channel < 3; ++channel)
          rgb[channel] = static_cast<int>(std::lround(color[channel] * 255.f));
        int lab[3];
        dcmqi::ColorUtilities::getIntegerScaledCIELabPCSFromSRGB(lab[0], lab[1], lab[2], rgb[0], rgb[1], rgb[2]);
        dcmqi::ColorUtilities::getSRGBFromIntegerScaledCIELabPCS(rgb[0], rgb[1], rgb[2], lab[0], lab[1], lab[2]);
        Color result;
        result.Set(rgb[0] / 255.f, rgb[1] / 255.f, rgb[2] / 255.f);
        return result;
      }

      inline void ReorderSingleGroup(MultiLabelSegmentation& segmentation,
                                     const MultiLabelSegmentation::LabelValueVectorType& values)
      {
        MultiLabelSegmentation::LabelVectorType labels;
        for (const auto value : values)
          labels.push_back(segmentation.GetLabel(value));
        segmentation.ReplaceGroupLabels(0, labels);
      }

      inline void RequireSingleGroup(const MultiLabelSegmentation& segmentation)
      {
        if (segmentation.GetNumberOfGroups() != 1)
          mitkThrow() << "The edit needs a single-group segmentation; this one has " << segmentation.GetNumberOfGroups() << " groups.";
      }
    }

    /**
     * Maps the label values of a single-group segmentation to their ranks 1..N, keeping the
     * order of the group. A DICOM SEG fixture built this way keeps its values on a round trip.
     */
    inline void RenumberToRanks(MultiLabelSegmentation& segmentation)
    {
      detail::RequireSingleGroup(segmentation);

      const auto groupValues = segmentation.GetLabelValuesByGroup(0);
      auto sortedValues = groupValues;
      std::sort(sortedValues.begin(), sortedValues.end());
      std::map<Label::PixelType, Label::PixelType> newValueByOldValue;
      for (std::size_t rank = 0; rank < sortedValues.size(); ++rank)
        newValueByOldValue[sortedValues[rank]] = static_cast<Label::PixelType>(rank + 1);

      MultiLabelSegmentation::LabelVectorType newLabels;
      for (const auto oldValue : groupValues)
      {
        auto label = segmentation.GetLabel(oldValue)->Clone();
        label->SetValue(newValueByOldValue.at(oldValue));
        newLabels.push_back(label);
      }

      {
        auto* groupImage = segmentation.GetGroupImage(0);
        {
          ImageWriteAccessor accessor(groupImage);
          auto* voxels = static_cast<Label::PixelType*>(accessor.GetData());
          const auto voxelCount = static_cast<std::size_t>(groupImage->GetDimension(0)) * groupImage->GetDimension(1) *
                                  groupImage->GetDimension(2) * groupImage->GetTimeSteps();
          for (std::size_t i = 0; i < voxelCount; ++i)
          {
            const auto finding = newValueByOldValue.find(voxels[i]);
            if (finding != newValueByOldValue.end())
              voxels[i] = finding->second;
          }
        }
      }
      segmentation.ReplaceGroupLabels(0, newLabels);
    }

    namespace detail
    {
      /** Wraps a label loss into a segmentation loss that applies it to every label. */
      inline DocumentedLoss ForEveryLabel(const DocumentedLabelLoss& loss)
      {
        return { loss.description, [apply = loss.apply](MultiLabelSegmentation& segmentation) {
                  for (const auto& label : segmentation.GetLabels())
                    apply(*label);
                } };
      }
    }

    /**
     * What DICOM SEG loses of a label through its upstream libraries, although the standard lets
     * the file carry it. The writer hands the segments to dcmqi, whose segment attributes hold
     * one anatomic region with one modifier, one type modifier and no primary anatomic structure,
     * and which drops the algorithm name of MANUAL segments. The reader gets the segments from
     * dcmqi as well, which reads them through DCMTK's segmentation classes; those keep one
     * anatomic region. Files of other producers lose the same on read.
     */
    inline std::vector<DocumentedLabelLoss> DICOMSegLabelLosses()
    {
      return {
        { "anatomic regions beyond the first: dcmqi writes one, and DCMTK's segmentation classes read one item of"
          " the Anatomic Region Sequence, which comes back at index 0",
          [](Label& label) {
            const auto indices =
              detail::FindLabelCodeIndices(label, LabelPropertyConstants::GetAnatomicRegionPropertyBaseName());
            if (indices.empty() || (indices.size() == 1 && indices.front() == 0))
              return;
            const auto first = label.GetAnatomicRegion(indices.front());
            for (const auto index : indices)
              label.RemoveAnatomicRegion(index);
            label.SetAnatomicRegion(first, 0);
          } },
        { "anatomic region modifiers beyond the first: dcmqi writes and reads only the first",
          [](Label& label) {
            for (const auto index :
                 detail::FindLabelCodeIndices(label, LabelPropertyConstants::GetAnatomicRegionPropertyBaseName()))
            {
              auto region = label.GetAnatomicRegion(index);
              auto modifiers = region.GetModifiers();
              if (modifiers.size() > 1)
              {
                modifiers.resize(1);
                region.SetModifiers(modifiers);
                label.SetAnatomicRegion(region, index);
              }
            }
          } },
        { "segmented property type modifiers beyond the first: dcmqi writes and reads only the first",
          [](Label& label) {
            auto type = label.GetSegmentedPropertyType();
            if (!type.has_value() || type->GetModifiers().size() <= 1)
              return;
            auto modifiers = type->GetModifiers();
            modifiers.resize(1);
            type->SetModifiers(modifiers);
            label.SetSegmentedPropertyType(*type);
          } },
        { "primary anatomic structures: dcmqi writes and reads none",
          [](Label& label) {
            for (const auto index :
                 detail::FindLabelCodeIndices(label, LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName()))
              label.RemovePrimaryAnatomicStructure(index);
          } },
        { "the algorithm name of MANUAL segments: dcmqi writes and reads it only for other algorithm types",
          [](Label& label) {
            if (label.GetAlgorithmType() == Label::AlgorithmType::MANUAL)
              label.SetAlgorithmName("");
          } },
      };
    }

    /** What DICOM SEG cannot carry of a label: it has no field for it, or a coarser one. */
    inline std::vector<DocumentedLabelLoss> DICOMSegLabelFormatLosses()
    {
      return {
        { "opacity, visible and locked: the format has no field for them; the reader's values are pinned",
          [](Label& label) {
            label.SetOpacity(0.6f);
            label.SetVisible(true);
            label.SetLocked(true);
          } },
        { "custom label properties: the format has no field for them",
          [](Label& label) {
            for (const auto& key : label.GetPropertyKeys())
            {
              if (!detail::IsDICOMSegLabelAttribute(key) && !detail::StartsWith(key, "center."))
                label.RemoveProperty(key);
            }
          } },
        { "colour: the format holds integer CIELab; the writer rounds to 8 bit sRGB, and dcmqi converts both ways",
          [](Label& label) { label.SetColor(detail::DICOMSegColorRoundTrip(label.GetColor())); } },
      };
    }

    /** What the DICOM SEG writer puts on the wire for an attribute the label lacks, because the format requires it. */
    inline void ApplyDICOMSegWriterDefaults(Label& label)
    {
      if (label.GetAlgorithmType() == Label::AlgorithmType::Undefined)
        label.SetAlgorithmType(Label::AlgorithmType::MANUAL);
      if (!label.GetSegmentedPropertyCategory().has_value())
        label.SetSegmentedPropertyCategory(DICOMCodeSequence("49755003", "SCT", "Morphologically altered structure"));
      if (!label.GetSegmentedPropertyType().has_value())
      {
        DICOMCodeSequenceWithModifiers type("49755003", "SCT", "Morphologically altered structure");
        type.AddModifier(DICOMCodeSequence("261665006", "SCT", "Unknown (qualifier value)"));
        label.SetSegmentedPropertyType(type);
      }
    }

    /**
     * Splits a segmentation into one single-group segmentation per group, which is what a DICOM
     * SEG write and the reload of its files yield. Each keeps the UID, properties and relations.
     */
    inline std::vector<MultiLabelSegmentation::ConstPointer> SplitIntoGroups(const MultiLabelSegmentation& segmentation)
    {
      std::vector<MultiLabelSegmentation::ConstPointer> result;
      for (MultiLabelSegmentation::GroupIndexType group = 0; group < segmentation.GetNumberOfGroups(); ++group)
      {
        auto single = CloneKeepingUID(segmentation);
        for (auto other = segmentation.GetNumberOfGroups(); other > 0; --other)
        {
          if (other - 1 != group)
            single->RemoveGroup(other - 1);
        }
        result.push_back(single.GetPointer());
      }
      return result;
    }

    /**
     * DICOM SEG, binary and labelmap (DICOMSegmentationIO). The writer writes every group to a
     * file of its own, so the expectations are single-group segmentations (SplitIntoGroups).
     * valuesAreOneToN drops the renumbering for fixtures whose values are already 1..N
     * (RenumberToRanks).
     */
    inline FormatCapabilities DICOMSegCapabilities(bool valuesAreOneToN)
    {
      FormatCapabilities capabilities;
      capabilities.formatName = valuesAreOneToN ? "DICOM SEG, values 1..N" : "DICOM SEG";

      // The reader mints a new segmentation UID and new relation UIDs.
      capabilities.options.notCompared = { NotCompared::SegmentationUID, NotCompared::RelationUIDs };
      // The colour loss reproduces the CIELab round trip exactly.
      capabilities.options.colorTolerance = 0.f;
      // dcmqi writes Image Position and Orientation (Patient) as float32 (Helper::floatToStr), which
      // rounds to 2^-24 relative; at the largest coordinate of the fixtures, 30.25 mm, that bounds
      // the error by 1.8e-6. Pixel Spacing carries 7 significant digits, 1.3e-6 at 2.5 mm.
      capabilities.options.geometryTolerance = 2e-6;
      // The format has no time; the reload has the same single, unbounded time step.
      capabilities.options.timeTolerance = 0.0;
      // The reader copies every tag registered with the process-global tags-of-interest service,
      // so which DICOM keys it adds depends on the loaded modules.
      capabilities.options.allowedAddedKeyPrefixes = { "DICOM." };

      capabilities.losses.push_back({
        "group names: DICOM SEG has no field meant for them, and every group is a file of its own (owner decision)",
        [](MultiLabelSegmentation& segmentation) {
          for (MultiLabelSegmentation::GroupIndexType group = 0; group < segmentation.GetNumberOfGroups(); ++group)
            segmentation.SetGroupName(group, "");
        } });
      capabilities.losses.push_back({
        "custom segmentation properties: the format has no field for them",
        [](MultiLabelSegmentation& segmentation) {
          auto properties = segmentation.GetPropertyList();
          for (const auto& key : properties->GetPropertyKeys())
          {
            if (!detail::HasDedicatedPersistence(key))
              properties->RemoveProperty(key);
          }
        } });
      capabilities.losses.push_back({
        "the unlabeled lock: the format has no field for it, and the reader leaves it unlocked",
        [](MultiLabelSegmentation& segmentation) { segmentation.SetUnlabeledLabelLock(false); } });
      capabilities.losses.push_back({
        "the order of the labels: the writer numbers the segments by label value, and the reader adds them in that order",
        [](MultiLabelSegmentation& segmentation) {
          detail::RequireSingleGroup(segmentation);
          auto values = segmentation.GetLabelValuesByGroup(0);
          std::sort(values.begin(), values.end());
          detail::ReorderSingleGroup(segmentation, values);
        } });
      if (!valuesAreOneToN)
      {
        capabilities.losses.push_back({
          "label values: segment numbers run 1..N, so every label gets its rank within the group",
          [](MultiLabelSegmentation& segmentation) { RenumberToRanks(segmentation); } });
      }

      for (const auto& loss : DICOMSegLabelLosses())
        capabilities.losses.push_back(detail::ForEveryLabel(loss));
      for (const auto& loss : DICOMSegLabelFormatLosses())
        capabilities.losses.push_back(detail::ForEveryLabel(loss));

      return capabilities;
    }
  }
}

#endif
