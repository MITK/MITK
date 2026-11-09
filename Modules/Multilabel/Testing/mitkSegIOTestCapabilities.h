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
  }
}

#endif
