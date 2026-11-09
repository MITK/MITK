/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSegFileContentTestHelpers_h
#define mitkDICOMSegFileContentTestHelpers_h

#include <mitkSegIOTestCapabilities.h>
#include <mitkSegIOTestComparator.h>

#include <mitkExceptionMacro.h>
#include <mitkImageReadAccessor.h>
#include <mitkLabelSetImage.h>

#include <dcmqi/ColorUtilities.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcsequen.h>

#include <itkMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace mitk
{
  namespace test
  {
    /** One segment of a DICOM SEG file, as the reader is expected to yield it. */
    struct DICOMSegFileSegment
    {
      /** The group the reader puts the segment into. */
      MultiLabelSegmentation::GroupIndexType group = 0;
      /** Every attribute of the Segment Sequence item; the label value is the segment number. */
      Label::Pointer label;
      /** The centres of the segment's voxels, in world coordinates. */
      std::vector<Point3D> voxels;
    };

    /**
     * The segments of a BINARY or LABELMAP DICOM SEG file, read with DCMTK's dcmdata layer.
     * That layer sees every element and sequence item of the file. The reader instead gets
     * the segments from dcmqi and DCMTK's segmentation classes, so this oracle shares
     * neither their limits nor their interpretation of the file.
     */
    struct DICOMSegFileContent
    {
      std::size_t groupCount = 0;
      /** In Segment Sequence order, without the background segment 0 of a LABELMAP. */
      std::vector<DICOMSegFileSegment> segments;
    };

    namespace detail
    {
      inline std::string GetDICOMString(DcmItem& item, const DcmTagKey& tag)
      {
        OFString value;
        item.findAndGetOFString(tag, value);
        return value.c_str();
      }

      inline std::vector<DcmItem*> GetSequenceItems(DcmItem& item, const DcmTagKey& sequence)
      {
        std::vector<DcmItem*> items;
        DcmSequenceOfItems* sequenceElement = nullptr;
        if (item.findAndGetSequence(sequence, sequenceElement).good() && nullptr != sequenceElement)
        {
          for (unsigned long i = 0; i < sequenceElement->card(); ++i)
            items.push_back(sequenceElement->getItem(i));
        }
        return items;
      }

      inline DICOMCodeSequence ReadDICOMCode(DcmItem& codeItem)
      {
        return DICOMCodeSequence(GetDICOMString(codeItem, DCM_CodeValue),
                                 GetDICOMString(codeItem, DCM_CodingSchemeDesignator),
                                 GetDICOMString(codeItem, DCM_CodeMeaning));
      }

      inline DICOMCodeSequenceWithModifiers ReadDICOMCodeWithModifiers(DcmItem& codeItem, const DcmTagKey& modifierSequence)
      {
        DICOMCodeSequenceWithModifiers code(ReadDICOMCode(codeItem));
        for (auto* modifier : GetSequenceItems(codeItem, modifierSequence))
          code.AddModifier(ReadDICOMCode(*modifier));
        return code;
      }

      inline Label::Pointer ReadSegmentAsLabel(DcmItem& segment, Uint16 number)
      {
        auto label = Label::New();
        label->SetValue(number);
        // The format carries no opacity, visibility or lock state; the reader's values are pinned.
        label->SetOpacity(0.6f);
        label->SetVisible(true);
        label->SetLocked(true);

        const auto name = GetDICOMString(segment, DCM_SegmentLabel);
        if (name.empty())
          mitkThrow() << "Segment " << number << " has no Segment Label, although it is Type 1.";
        label->SetName(name);

        // dcmqi hands the reader the colour as integer sRGB converted from the CIELab value,
        // and substitutes this value when the item has none (Dicom2ItkConverterBase).
        Uint16 lab[3] = { 43803, 26565, 37722 };
        if (segment.tagExistsWithValue(DCM_RecommendedDisplayCIELabValue))
        {
          for (unsigned long i = 0; i < 3; ++i)
            segment.findAndGetUint16(DCM_RecommendedDisplayCIELabValue, lab[i], i);
        }
        int rgb[3] = { 0, 0, 0 };
        dcmqi::ColorUtilities::getSRGBFromIntegerScaledCIELabPCS(rgb[0], rgb[1], rgb[2], lab[0], lab[1], lab[2]);
        Color color;
        color.Set(rgb[0] / 255.f, rgb[1] / 255.f, rgb[2] / 255.f);
        label->SetColor(color);

        const auto description = GetDICOMString(segment, DCM_SegmentDescription);
        if (!description.empty())
          label->SetDescription(description);

        const auto algorithmType = GetDICOMString(segment, DCM_SegmentAlgorithmType);
        if (!algorithmType.empty())
          label->SetAlgorithmTypeStr(algorithmType);
        const auto algorithmName = GetDICOMString(segment, DCM_SegmentAlgorithmName);
        if (!algorithmName.empty())
          label->SetAlgorithmName(algorithmName);

        const auto categories = GetSequenceItems(segment, DCM_SegmentedPropertyCategoryCodeSequence);
        if (!categories.empty())
          label->SetSegmentedPropertyCategory(ReadDICOMCode(*categories.front()));

        const auto types = GetSequenceItems(segment, DCM_SegmentedPropertyTypeCodeSequence);
        if (!types.empty())
          label->SetSegmentedPropertyType(
            ReadDICOMCodeWithModifiers(*types.front(), DCM_SegmentedPropertyTypeModifierCodeSequence));

        std::size_t index = 0;
        for (auto* region : GetSequenceItems(segment, DCM_AnatomicRegionSequence))
          label->SetAnatomicRegion(ReadDICOMCodeWithModifiers(*region, DCM_AnatomicRegionModifierSequence), index++);

        index = 0;
        for (auto* structure : GetSequenceItems(segment, DCM_PrimaryAnatomicStructureSequence))
        {
          label->SetPrimaryAnatomicStructure(
            ReadDICOMCodeWithModifiers(*structure, DCM_PrimaryAnatomicStructureModifierSequence), index++);
        }

        const auto trackingID = GetDICOMString(segment, DCM_TrackingID);
        if (!trackingID.empty())
          label->SetTrackingID(trackingID);
        const auto trackingUID = GetDICOMString(segment, DCM_TrackingUID);
        if (!trackingUID.empty())
          label->SetTrackingUID(trackingUID);

        return label;
      }

      /** The functional group item of a frame: its own if it has one, else the shared one. */
      inline DcmItem* FindFunctionalGroup(DcmItem* perFrame, DcmItem* shared, const DcmTagKey& sequence)
      {
        for (auto* source : { perFrame, shared })
        {
          DcmItem* item = nullptr;
          if (nullptr != source && source->findAndGetSequenceItem(sequence, item, 0).good() && nullptr != item)
            return item;
        }
        return nullptr;
      }

      inline Float64 GetDICOMDouble(DcmItem* item, const DcmTagKey& tag, unsigned long position, const std::string& context)
      {
        Float64 value = 0.0;
        if (nullptr == item || item->findAndGetFloat64(tag, value, position).bad())
          mitkThrow() << context << ": " << DcmTag(tag).getTagName() << " is missing.";
        return value;
      }

      // Each frame carries its own Image Position (Patient), written with at least 7 significant
      // digits by the producers of the fixtures (dcmqi's float32 strings, highdicom copying the
      // 6-decimal strings of its source), so at most 5e-7 relative. A frame can thus lie off the
      // grid that starts at the first frame by twice that: 2 x 5e-7 x 235.2 mm / 0.81 mm = 2.9e-4
      // voxel for the largest fixture coordinate. The 7 digits of the slice spacing and the float32
      // orientation vectors add less than 5e-5. A reader that places the voxels off by more is wrong.
      constexpr double VOXEL_CENTRE_TOLERANCE = 1e-3;

      inline void CompareSegmentVoxels(const MultiLabelSegmentation& actual,
                                       const DICOMSegFileSegment& segment,
                                       std::vector<std::string>& differences)
      {
        const auto value = segment.label->GetValue();
        const auto context = DescribeLabel(segment.label) + " of the file";
        const auto* image = actual.GetGroupImage(segment.group);
        const auto* geometry = image->GetGeometry();
        const std::size_t dimX = image->GetDimension(0);
        const std::size_t dimY = image->GetDimension(1);
        const std::size_t voxelCount = dimX * dimY * image->GetDimension(2);

        ImageReadAccessor accessor(image);
        const auto* voxels = static_cast<const Label::PixelType*>(accessor.GetData());

        std::set<std::size_t> offsets;
        std::size_t outside = 0;
        std::size_t otherValue = 0;
        std::size_t offCentre = 0;
        double largestDistance = 0.0;
        for (const auto& point : segment.voxels)
        {
          Point3D continuousIndex;
          geometry->WorldToIndex(point, continuousIndex);
          itk::Index<3> index;
          double distance = 0.0;
          for (unsigned int axis = 0; axis < 3; ++axis)
          {
            index[axis] = itk::Math::RoundHalfIntegerUp<itk::IndexValueType>(continuousIndex[axis]);
            distance = std::max(distance, std::abs(continuousIndex[axis] - static_cast<double>(index[axis])));
          }
          if (distance > VOXEL_CENTRE_TOLERANCE)
          {
            ++offCentre;
            largestDistance = std::max(largestDistance, distance);
          }
          if (!geometry->IsIndexInside(index))
          {
            ++outside;
            continue;
          }
          const auto offset = static_cast<std::size_t>(index[0]) + static_cast<std::size_t>(index[1]) * dimX +
                              static_cast<std::size_t>(index[2]) * dimX * dimY;
          if (voxels[offset] != value)
            ++otherValue;
          offsets.insert(offset);
        }

        const auto ofFileVoxels = " of " + std::to_string(segment.voxels.size()) + " file voxels";
        if (outside > 0)
          differences.push_back(context + ": " + std::to_string(outside) + ofFileVoxels + " lie outside the group image");
        if (offCentre > 0)
        {
          differences.push_back(context + ": " + std::to_string(offCentre) + ofFileVoxels +
                                " lie off the voxel centres of the group image, by up to " +
                                std::to_string(largestDistance) + " voxel");
        }
        if (otherValue > 0)
          differences.push_back(context + ": " + std::to_string(otherValue) + ofFileVoxels + " carry another value");
        if (offsets.size() + outside != segment.voxels.size())
          differences.push_back(context + ": several file voxels fall onto the same image voxel");

        const auto labelled = static_cast<std::size_t>(std::count(voxels, voxels + voxelCount, value));
        if (labelled != offsets.size())
        {
          differences.push_back(context + ": the group image has " + std::to_string(labelled) +
                                " voxels with the value, the file " + std::to_string(segment.voxels.size()));
        }
      }

      // CompareSegmentVoxels finds the voxels of each segment; this finds every other value.
      inline void CompareVoxelsOutsideSegments(const MultiLabelSegmentation& actual,
                                               MultiLabelSegmentation::GroupIndexType group,
                                               MultiLabelSegmentation::LabelValueVectorType segmentValues,
                                               std::vector<std::string>& differences)
      {
        std::sort(segmentValues.begin(), segmentValues.end());
        const auto* image = actual.GetGroupImage(group);
        const std::size_t voxelCount =
          static_cast<std::size_t>(image->GetDimension(0)) * image->GetDimension(1) * image->GetDimension(2);

        ImageReadAccessor accessor(image);
        const auto* voxels = static_cast<const Label::PixelType*>(accessor.GetData());

        std::map<Label::PixelType, std::size_t> strayCounts;
        for (std::size_t offset = 0; offset < voxelCount; ++offset)
        {
          const auto value = voxels[offset];
          if (value != MultiLabelSegmentation::UNLABELED_VALUE &&
              !std::binary_search(segmentValues.begin(), segmentValues.end(), value))
            ++strayCounts[value];
        }

        for (const auto& [value, count] : strayCounts)
        {
          differences.push_back("group " + std::to_string(group) + ": " + std::to_string(count) + " of " +
                                std::to_string(voxelCount) + " voxels carry the value " + std::to_string(value) +
                                ", which no segment of the group has");
        }
      }
    }

    /**
     * Reads what the reader is expected to yield from a BINARY or LABELMAP DICOM SEG file.
     * The group layout follows the reader's contract: a LABELMAP is one group, a BINARY SEG
     * is one group if SegmentsOverlap is NO and otherwise one group per segment, in segment
     * order. Throws mitk::Exception for input it does not model, such as a segment without
     * voxels, so that a test cannot pass on an empty oracle.
     */
    inline DICOMSegFileContent ReadDICOMSegFileContent(const std::string& path)
    {
      DcmFileFormat file;
      if (file.loadFile(path.c_str()).bad())
        mitkThrow() << "DCMTK cannot read " << path << ".";
      auto& dataset = *file.getDataset();

      const auto segmentationType = detail::GetDICOMString(dataset, DCM_SegmentationType);
      if (segmentationType != "BINARY" && segmentationType != "LABELMAP")
        mitkThrow() << path << " has the segmentation type '" << segmentationType << "'; only BINARY and LABELMAP are read.";
      const bool labelmap = segmentationType == "LABELMAP";
      const bool sharedGroup = labelmap || detail::GetDICOMString(dataset, DCM_SegmentsOverlap) == "NO";

      DICOMSegFileContent content;
      std::map<Uint16, std::size_t> segmentIndexByNumber;
      for (auto* item : detail::GetSequenceItems(dataset, DCM_SegmentSequence))
      {
        Uint16 number = 0;
        if (item->findAndGetUint16(DCM_SegmentNumber, number).bad())
          mitkThrow() << path << " has a segment without Segment Number.";
        // A labelmap may describe its pixel value 0, which the reader keeps unlabeled.
        if (labelmap && 0 == number)
          continue;

        DICOMSegFileSegment segment;
        segment.group = sharedGroup ? 0 : static_cast<MultiLabelSegmentation::GroupIndexType>(content.segments.size());
        segment.label = detail::ReadSegmentAsLabel(*item, number);
        segmentIndexByNumber[number] = content.segments.size();
        content.segments.push_back(segment);
      }
      if (content.segments.empty())
        mitkThrow() << path << " has no segments.";
      content.groupCount = sharedGroup ? 1 : content.segments.size();

      Uint16 rows = 0;
      Uint16 columns = 0;
      Uint16 bitsAllocated = 0;
      Sint32 frameCount = 0;
      dataset.findAndGetUint16(DCM_Rows, rows);
      dataset.findAndGetUint16(DCM_Columns, columns);
      dataset.findAndGetUint16(DCM_BitsAllocated, bitsAllocated);
      dataset.findAndGetSint32(DCM_NumberOfFrames, frameCount);
      const std::size_t pixelsPerFrame = static_cast<std::size_t>(rows) * columns;
      const std::size_t pixelCount = pixelsPerFrame * static_cast<std::size_t>(frameCount);
      if (pixelCount == 0)
        mitkThrow() << path << " has no pixels.";

      const Uint8* bytes = nullptr;
      const Uint16* words = nullptr;
      unsigned long elementCount = 0;
      bool pixelsRead = false;
      if (!labelmap && bitsAllocated == 1)
        pixelsRead = dataset.findAndGetUint8Array(DCM_PixelData, bytes, &elementCount).good() && elementCount * 8 >= pixelCount;
      else if (labelmap && bitsAllocated == 8)
        pixelsRead = dataset.findAndGetUint8Array(DCM_PixelData, bytes, &elementCount).good() && elementCount >= pixelCount;
      else if (labelmap && bitsAllocated == 16)
        pixelsRead = dataset.findAndGetUint16Array(DCM_PixelData, words, &elementCount).good() && elementCount >= pixelCount;
      if (!pixelsRead)
        mitkThrow() << path << ": cannot read " << pixelCount << " uncompressed pixels of " << bitsAllocated << " bits.";

      DcmItem* shared = nullptr;
      dataset.findAndGetSequenceItem(DCM_SharedFunctionalGroupsSequence, shared, 0);
      const auto perFrameGroups = detail::GetSequenceItems(dataset, DCM_PerFrameFunctionalGroupsSequence);
      if (perFrameGroups.size() != static_cast<std::size_t>(frameCount))
        mitkThrow() << path << " has " << perFrameGroups.size() << " per-frame functional groups for " << frameCount << " frames.";

      for (std::size_t frame = 0; frame < perFrameGroups.size(); ++frame)
      {
        auto* perFrame = perFrameGroups[frame];
        const auto context = path + ", frame " + std::to_string(frame);
        auto* position = detail::FindFunctionalGroup(perFrame, shared, DCM_PlanePositionSequence);
        auto* orientation = detail::FindFunctionalGroup(perFrame, shared, DCM_PlaneOrientationSequence);
        auto* measures = detail::FindFunctionalGroup(perFrame, shared, DCM_PixelMeasuresSequence);

        Point3D origin;
        Vector3D columnStep;
        Vector3D rowStep;
        for (unsigned int axis = 0; axis < 3; ++axis)
        {
          origin[axis] = detail::GetDICOMDouble(position, DCM_ImagePositionPatient, axis, context);
          // The first direction of Image Orientation (Patient) runs along a row, i.e. across the
          // columns, so it is scaled by the column spacing, the second value of Pixel Spacing.
          columnStep[axis] = detail::GetDICOMDouble(orientation, DCM_ImageOrientationPatient, axis, context) *
                             detail::GetDICOMDouble(measures, DCM_PixelSpacing, 1, context);
          rowStep[axis] = detail::GetDICOMDouble(orientation, DCM_ImageOrientationPatient, axis + 3, context) *
                          detail::GetDICOMDouble(measures, DCM_PixelSpacing, 0, context);
        }

        std::size_t binarySegmentIndex = 0;
        if (!labelmap)
        {
          DcmItem* identification = nullptr;
          Uint16 number = 0;
          if (nullptr == perFrame ||
              perFrame->findAndGetSequenceItem(DCM_SegmentIdentificationSequence, identification, 0).bad() ||
              identification->findAndGetUint16(DCM_ReferencedSegmentNumber, number).bad())
            mitkThrow() << context << " references no segment.";
          const auto finding = segmentIndexByNumber.find(number);
          if (finding == segmentIndexByNumber.end())
            mitkThrow() << context << " references segment " << number << ", which the Segment Sequence lacks.";
          binarySegmentIndex = finding->second;
        }

        for (std::size_t row = 0; row < rows; ++row)
        {
          for (std::size_t column = 0; column < columns; ++column)
          {
            const auto pixel = frame * pixelsPerFrame + row * columns + column;
            std::size_t segmentIndex = binarySegmentIndex;
            if (labelmap)
            {
              const Uint16 pixelValue = nullptr != words ? words[pixel] : bytes[pixel];
              if (0 == pixelValue)
                continue;
              const auto finding = segmentIndexByNumber.find(pixelValue);
              if (finding == segmentIndexByNumber.end())
                mitkThrow() << context << " has the pixel value " << pixelValue << ", which no segment describes.";
              segmentIndex = finding->second;
            }
            else if (0 == ((bytes[pixel / 8] >> (pixel % 8)) & 1))
            {
              // BINARY frames are bit-packed, the first pixel in the least significant bit.
              continue;
            }

            content.segments[segmentIndex].voxels.push_back(origin + columnStep * static_cast<double>(column) +
                                                            rowStep * static_cast<double>(row));
          }
        }
      }

      for (const auto& segment : content.segments)
      {
        if (segment.voxels.empty())
          mitkThrow() << path << ": segment " << segment.label->GetValue() << " has no voxels, which this oracle does not model.";
      }

      return content;
    }

    /**
     * Compares a segmentation that the reader made from a DICOM SEG file with the file's
     * own content and returns one readable line per difference; an empty result means equal.
     * Per segment: the label with the segment number as value lies in the expected group,
     * its properties equal the segment's attributes after the documented read losses
     * (DICOMSegLabelLosses), compared by key, type and value, and its voxels are exactly
     * the segment's, each file voxel centre on a voxel centre of the group image within the
     * rounding of the file's positions. The group count, the label values of each group in
     * segment order, labels without a segment, and voxels of a group image that no segment of
     * the group has are checked too.
     */
    inline std::vector<std::string> CompareWithDICOMSegFileContent(const MultiLabelSegmentation& actual,
                                                                   const DICOMSegFileContent& content)
    {
      std::vector<std::string> differences;

      if (actual.GetNumberOfGroups() != content.groupCount)
      {
        differences.push_back("group count: expected " + std::to_string(content.groupCount) + ", actual " +
                              std::to_string(actual.GetNumberOfGroups()));
      }

      std::vector<MultiLabelSegmentation::LabelValueVectorType> expectedValuesByGroup(content.groupCount);
      for (const auto& segment : content.segments)
        expectedValuesByGroup[segment.group].push_back(segment.label->GetValue());
      const auto groupCount = std::min<std::size_t>(content.groupCount, actual.GetNumberOfGroups());
      for (std::size_t group = 0; group < groupCount; ++group)
      {
        const auto actualValues = actual.GetLabelValuesByGroup(static_cast<MultiLabelSegmentation::GroupIndexType>(group));
        if (actualValues != expectedValuesByGroup[group])
        {
          differences.push_back("group " + std::to_string(group) + ": label values expected " +
                                detail::DescribeValues(expectedValuesByGroup[group]) + ", actual " +
                                detail::DescribeValues(actualValues));
        }
        detail::CompareVoxelsOutsideSegments(actual, static_cast<MultiLabelSegmentation::GroupIndexType>(group),
                                             expectedValuesByGroup[group], differences);
      }

      const auto losses = DICOMSegLabelLosses();
      std::set<Label::PixelType> segmentValues;
      for (const auto& segment : content.segments)
      {
        const auto value = segment.label->GetValue();
        segmentValues.insert(value);
        const auto context = detail::DescribeLabel(segment.label);
        if (!actual.ExistLabel(value))
        {
          differences.push_back(context + ": missing");
          continue;
        }
        if (actual.GetGroupIndexOfLabel(value) != segment.group)
        {
          differences.push_back(context + ": in group " + std::to_string(actual.GetGroupIndexOfLabel(value)) +
                                ", expected " + std::to_string(segment.group));
          continue;
        }

        auto expected = segment.label->Clone();
        for (const auto& loss : losses)
          loss.apply(*expected);
        detail::CompareLabelProperties(*expected, *actual.GetLabel(value), 0.f, differences);
        detail::CompareSegmentVoxels(actual, segment, differences);
      }

      for (const auto value : actual.GetAllLabelValues())
      {
        if (segmentValues.count(value) == 0)
          differences.push_back(detail::DescribeLabel(actual.GetLabel(value)) + ": unexpected, no segment has this value");
      }

      return differences;
    }

    /** Reads the file's content and compares the segmentation with it, see CompareWithDICOMSegFileContent. */
    inline std::vector<std::string> CompareWithDICOMSegFile(const MultiLabelSegmentation& actual, const std::string& path)
    {
      return CompareWithDICOMSegFileContent(actual, ReadDICOMSegFileContent(path));
    }

    /** The items of a Segment Sequence, keyed by Segment Number. */
    using DICOMSegSegmentMap = std::map<Uint16, Label::Pointer>;

    /**
     * Reads every item of the Segment Sequence of a DICOM SEG file, as a label like
     * ReadDICOMSegFileContent does, but without voxels and including the background segment 0
     * of a LABELMAP. Throws mitk::Exception for an item without or with a repeated Segment Number.
     */
    inline DICOMSegSegmentMap ReadSegmentSequence(const std::string& path)
    {
      DcmFileFormat file;
      if (file.loadFile(path.c_str()).bad())
        mitkThrow() << "DCMTK cannot read " << path << ".";

      DICOMSegSegmentMap segments;
      for (auto* item : detail::GetSequenceItems(*file.getDataset(), DCM_SegmentSequence))
      {
        Uint16 number = 0;
        if (item->findAndGetUint16(DCM_SegmentNumber, number).bad())
          mitkThrow() << path << " has a segment without Segment Number.";
        if (!segments.emplace(number, detail::ReadSegmentAsLabel(*item, number)).second)
          mitkThrow() << path << " has two segments with the Segment Number " << number << ".";
      }
      return segments;
    }

    /** The Recommended Display CIELab Value of every segment of a DICOM SEG file that has one, keyed by Segment Number. */
    inline std::map<Uint16, std::array<Uint16, 3>> ReadRecommendedDisplayCIELab(const std::string& path)
    {
      DcmFileFormat file;
      if (file.loadFile(path.c_str()).bad())
        mitkThrow() << "DCMTK cannot read " << path << ".";

      std::map<Uint16, std::array<Uint16, 3>> result;
      for (auto* item : detail::GetSequenceItems(*file.getDataset(), DCM_SegmentSequence))
      {
        Uint16 number = 0;
        item->findAndGetUint16(DCM_SegmentNumber, number);
        if (!item->tagExistsWithValue(DCM_RecommendedDisplayCIELabValue))
          continue;
        std::array<Uint16, 3> lab = { 0, 0, 0 };
        for (unsigned long i = 0; i < 3; ++i)
          item->findAndGetUint16(DCM_RecommendedDisplayCIELabValue, lab[i], i);
        result[number] = lab;
      }
      return result;
    }

    /**
     * The Segment Sequence the DICOM SEG writer is expected to write for one group of a
     * segmentation, as ReadSegmentSequence reads it: the labels in ascending value order, each
     * numbered by its rank, with the writer's defaults for required attributes the label lacks
     * and after the documented label losses. A LABELMAP has the background segment 0 that dcmqi
     * adds when the group image has unlabeled voxels.
     */
    inline DICOMSegSegmentMap ExpectedSegmentSequence(const MultiLabelSegmentation& segmentation,
                                                      MultiLabelSegmentation::GroupIndexType group,
                                                      bool labelmap)
    {
      auto values = segmentation.GetLabelValuesByGroup(group);
      std::sort(values.begin(), values.end());

      DICOMSegSegmentMap segments;
      for (const auto value : values)
      {
        auto label = segmentation.GetLabel(value)->Clone();
        ApplyDICOMSegWriterDefaults(*label);
        for (const auto& loss : DICOMSegLabelLosses())
          loss.apply(*label);
        for (const auto& loss : DICOMSegLabelFormatLosses())
          loss.apply(*label);
        const auto number = static_cast<Uint16>(segments.size() + 1);
        label->SetValue(number);
        segments[number] = label;
      }

      if (labelmap)
      {
        const auto* image = segmentation.GetGroupImage(group);
        ImageReadAccessor accessor(image);
        const auto* voxels = static_cast<const Label::PixelType*>(accessor.GetData());
        const auto voxelCount = static_cast<std::size_t>(image->GetDimension(0)) * image->GetDimension(1) * image->GetDimension(2);
        if (std::find(voxels, voxels + voxelCount, MultiLabelSegmentation::UNLABELED_VALUE) != voxels + voxelCount)
        {
          auto background = Label::New(0, "Background");
          background->SetOpacity(0.6f);
          background->SetVisible(true);
          background->SetLocked(true);
          int rgb[3] = { 0, 0, 0 };
          dcmqi::ColorUtilities::getSRGBFromIntegerScaledCIELabPCS(rgb[0], rgb[1], rgb[2], 0, 32768, 32768);
          Color color;
          color.Set(rgb[0] / 255.f, rgb[1] / 255.f, rgb[2] / 255.f);
          background->SetColor(color);
          background->SetAlgorithmType(Label::AlgorithmType::AUTOMATIC);
          background->SetAlgorithmName("dcmqi");
          background->SetSegmentedPropertyCategory(DICOMCodeSequence("125040", "DCM", "Background"));
          background->SetSegmentedPropertyType(DICOMCodeSequenceWithModifiers("125040", "DCM", "Background"));
          segments[0] = background;
        }
      }

      return segments;
    }

    /** Compares two Segment Sequences segment by segment, by number, and returns one readable line per difference. */
    inline std::vector<std::string> CompareSegmentSequences(const DICOMSegSegmentMap& expected, const DICOMSegSegmentMap& actual)
    {
      std::vector<std::string> differences;
      for (const auto& [number, label] : expected)
      {
        const auto finding = actual.find(number);
        if (finding == actual.end())
        {
          differences.push_back("segment " + std::to_string(number) + " ('" + label->GetName() + "'): missing");
          continue;
        }
        std::vector<std::string> labelDifferences;
        detail::CompareLabelProperties(*label, *finding->second, 0.f, labelDifferences);
        for (const auto& difference : labelDifferences)
          differences.push_back("segment " + std::to_string(number) + ", " + difference);
      }
      for (const auto& [number, label] : actual)
      {
        if (expected.count(number) == 0)
          differences.push_back("segment " + std::to_string(number) + " ('" + label->GetName() + "'): unexpected");
      }
      return differences;
    }

    /**
     * Checks the file the DICOM SEG writer wrote for one group of a segmentation: every item of
     * its Segment Sequence against ExpectedSegmentSequence, and SegmentsOverlap NO, since the
     * labels of a group never overlap.
     */
    inline std::vector<std::string> CompareWrittenSegments(const std::string& path,
                                                           const MultiLabelSegmentation& segmentation,
                                                           MultiLabelSegmentation::GroupIndexType group,
                                                           bool labelmap)
    {
      auto differences = CompareSegmentSequences(ExpectedSegmentSequence(segmentation, group, labelmap), ReadSegmentSequence(path));

      DcmFileFormat file;
      if (file.loadFile(path.c_str()).bad())
        mitkThrow() << "DCMTK cannot read " << path << ".";
      const auto overlap = detail::GetDICOMString(*file.getDataset(), DCM_SegmentsOverlap);
      if (overlap != "NO")
        differences.push_back("SegmentsOverlap: expected 'NO', actual '" + overlap + "'");

      return differences;
    }
  }
}

#endif
