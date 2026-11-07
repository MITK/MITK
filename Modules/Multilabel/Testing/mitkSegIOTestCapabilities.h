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
#include <mitkStringProperty.h>

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

    /**
     * What the DICOM SEG reader cannot take from a segment, although the standard lets the
     * file carry it: the reader gets the segments from dcmqi, which reads them through
     * DCMTK's segmentation classes. Both are upstream limits.
     */
    inline std::vector<DocumentedLabelLoss> DICOMSegReadLabelLosses()
    {
      return {
        { "anatomic regions beyond the first: DCMTK's segmentation classes keep one item of the Anatomic Region Sequence",
          [](Label& label) {
            while (label.GetAnatomicRegionCount() > 1)
              label.RemoveAnatomicRegion(label.GetAnatomicRegionCount() - 1);
          } },
        { "anatomic region modifiers beyond the first: dcmqi reads only the first",
          [](Label& label) {
            if (label.GetAnatomicRegionCount() == 0)
              return;
            auto region = label.GetAnatomicRegion(0);
            auto modifiers = region.GetModifiers();
            if (modifiers.size() > 1)
            {
              modifiers.resize(1);
              region.SetModifiers(modifiers);
              label.SetAnatomicRegion(region, 0);
            }
          } },
        { "segmented property type modifiers beyond the first: dcmqi reads only the first",
          [](Label& label) {
            auto type = label.GetSegmentedPropertyType();
            if (!type.has_value() || type->GetModifiers().size() <= 1)
              return;
            auto modifiers = type->GetModifiers();
            modifiers.resize(1);
            type->SetModifiers(modifiers);
            label.SetSegmentedPropertyType(*type);
          } },
        { "primary anatomic structures: dcmqi reads none",
          [](Label& label) {
            while (label.GetPrimaryAnatomicStructureCount() > 0)
              label.RemovePrimaryAnatomicStructure(label.GetPrimaryAnatomicStructureCount() - 1);
          } },
        { "the algorithm name of MANUAL segments: dcmqi reads it only for other algorithm types",
          [](Label& label) {
            if (label.GetAlgorithmType() == Label::AlgorithmType::MANUAL)
              label.SetAlgorithmName("");
          } },
      };
    }
  }
}

#endif
