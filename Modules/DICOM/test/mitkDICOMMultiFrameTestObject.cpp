/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMMultiFrameTestObject.h"

#include <mitkException.h>
#include <mitkExceptionMacro.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcuid.h>

#include <algorithm>
#include <locale>
#include <sstream>

namespace
{
  /** DICOM decimal and integer strings are locale independent, so they are never
      formatted with the stream's ambient locale. */
  std::string ToDicomString(double value)
  {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << value;
    return stream.str();
  }

  std::string ToDicomString(unsigned int value)
  {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << value;
    return stream.str();
  }

  std::string GenerateUID()
  {
    char uid[100];
    return std::string(dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT));
  }

  void Require(const OFCondition& condition, const std::string& what)
  {
    if (condition.bad())
    {
      mitkThrow() << "Cannot build DICOM test object. " << what << ": " << condition.text();
    }
  }

  /** Appends a new item to the sequence, creating the sequence if needed. */
  DcmItem& AppendItem(DcmItem& parent, const DcmTagKey& sequenceTag)
  {
    DcmItem* item = nullptr;
    Require(parent.findOrCreateSequenceItem(sequenceTag, item, -2), "Cannot create sequence item");
    return *item;
  }

  void WritePixelValueTransformation(DcmItem& parent, double slope, double intercept)
  {
    DcmItem& item = AppendItem(parent, DCM_PixelValueTransformationSequence);
    Require(item.putAndInsertString(DCM_RescaleIntercept, ToDicomString(intercept).c_str()), "Rescale Intercept");
    Require(item.putAndInsertString(DCM_RescaleSlope, ToDicomString(slope).c_str()), "Rescale Slope");
    Require(item.putAndInsertString(DCM_RescaleType, "BQML"), "Rescale Type");
  }

  std::string PositionString(double z)
  {
    return "0\\0\\" + ToDicomString(z);
  }
}

mitk::DICOMMultiFrameTestObject mitk::DICOMMultiFrameTestObject::EnhancedPET(unsigned int frameCount)
{
  DICOMMultiFrameTestObject result;
  result.frames.reserve(frameCount);

  for (unsigned int k = 0; k < frameCount; ++k)
  {
    DICOMMultiFrameTestFrame frame;
    frame.constant = static_cast<std::int16_t>(100 + k);
    frame.inStackPosition = k + 1;
    result.frames.push_back(frame);
  }

  return result;
}

std::string mitk::DICOMMultiFrameTestObject::Write(const std::string& directory,
                                                   const std::string& filename) const
{
  if (this->frames.empty())
  {
    mitkThrow() << "Cannot build DICOM test object without frames.";
  }

  DcmFileFormat fileFormat;
  DcmDataset& dataset = *fileFormat.getDataset();

  const std::string studyUID = this->studyInstanceUID.empty() ? GenerateUID() : this->studyInstanceUID;
  const std::string seriesUID = this->seriesInstanceUID.empty() ? GenerateUID() : this->seriesInstanceUID;
  const std::string sopClassUID =
    !this->sopClassUID.empty() ? this->sopClassUID
                               : (this->functionalGroups ? UID_EnhancedPETImageStorage
                                                         : UID_NuclearMedicineImageStorage);
  const std::string modality =
    !this->modality.empty() ? this->modality : (this->functionalGroups ? "PT" : "NM");

  Require(dataset.putAndInsertString(DCM_SOPClassUID, sopClassUID.c_str()), "SOP Class UID");
  Require(dataset.putAndInsertString(DCM_SOPInstanceUID, GenerateUID().c_str()), "SOP Instance UID");
  Require(dataset.putAndInsertString(DCM_StudyInstanceUID, studyUID.c_str()), "Study Instance UID");
  Require(dataset.putAndInsertString(DCM_SeriesInstanceUID, seriesUID.c_str()), "Series Instance UID");
  Require(dataset.putAndInsertString(DCM_FrameOfReferenceUID, studyUID.c_str()), "Frame of Reference UID");
  Require(dataset.putAndInsertString(DCM_Modality, modality.c_str()), "Modality");
  Require(dataset.putAndInsertString(DCM_SeriesNumber, "1"), "Series Number");
  Require(dataset.putAndInsertString(DCM_InstanceNumber, ToDicomString(this->instanceNumber).c_str()),
          "Instance Number");
  Require(dataset.putAndInsertString(DCM_PatientName, "MULTIFRAME^TEST"), "Patient Name");
  Require(dataset.putAndInsertString(DCM_PatientID, "MULTIFRAME_TEST"), "Patient ID");

  Require(dataset.putAndInsertUint16(DCM_SamplesPerPixel, 1), "Samples per Pixel");
  Require(dataset.putAndInsertString(DCM_PhotometricInterpretation, "MONOCHROME2"), "Photometric Interpretation");
  Require(dataset.putAndInsertUint16(DCM_Rows, static_cast<Uint16>(this->rows)), "Rows");
  Require(dataset.putAndInsertUint16(DCM_Columns, static_cast<Uint16>(this->columns)), "Columns");
  Require(dataset.putAndInsertUint16(DCM_BitsAllocated, 16), "Bits Allocated");
  Require(dataset.putAndInsertUint16(DCM_BitsStored, 16), "Bits Stored");
  Require(dataset.putAndInsertUint16(DCM_HighBit, 15), "High Bit");
  Require(dataset.putAndInsertUint16(DCM_PixelRepresentation, 1), "Pixel Representation");
  Require(dataset.putAndInsertString(DCM_NumberOfFrames, ToDicomString(static_cast<unsigned int>(this->frames.size())).c_str()),
          "Number of Frames");

  if (this->functionalGroups)
  {
    // A conformant functional-group object carries geometry and rescale in the
    // macros only, which is what sends EquiDistantBlocksSorter, GetPixelSpacing
    // and GDCM's ComputeZSpacingFromIPP down the same fallback paths the IBSI
    // reference objects take.
    DcmItem& shared = AppendItem(dataset, DCM_SharedFunctionalGroupsSequence);

    DcmItem& orientation = AppendItem(shared, DCM_PlaneOrientationSequence);
    Require(orientation.putAndInsertString(DCM_ImageOrientationPatient, "1\\0\\0\\0\\1\\0"),
            "Image Orientation (Patient)");

    DcmItem& measures = AppendItem(shared, DCM_PixelMeasuresSequence);
    Require(measures.putAndInsertString(DCM_SliceThickness, ToDicomString(this->sliceSpacing).c_str()),
            "Slice Thickness");
    const std::string spacing = ToDicomString(this->sliceSpacing) + "\\" + ToDicomString(this->sliceSpacing);
    Require(measures.putAndInsertString(DCM_PixelSpacing, spacing.c_str()), "Pixel Spacing");

    if (this->rescaleInSharedGroup)
    {
      WritePixelValueTransformation(shared, this->frames.front().slope, this->frames.front().intercept);
    }

    if (this->topLevelGeometry)
    {
      Require(dataset.putAndInsertString(DCM_ImageOrientationPatient, "1\\0\\0\\0\\1\\0"),
              "Image Orientation (Patient)");
      Require(dataset.putAndInsertString(DCM_ImagePositionPatient, PositionString(this->zOffset).c_str()),
              "Image Position (Patient)");
      Require(dataset.putAndInsertString(DCM_PixelSpacing, spacing.c_str()), "Pixel Spacing");
    }

    const unsigned int itemCount = this->perFrameItemCountOverride > 0
                                     ? this->perFrameItemCountOverride
                                     : static_cast<unsigned int>(this->frames.size());

    for (unsigned int k = 0; k < itemCount; ++k)
    {
      const DICOMMultiFrameTestFrame& frame = this->frames[std::min<std::size_t>(k, this->frames.size() - 1)];
      DcmItem& perFrame = AppendItem(dataset, DCM_PerFrameFunctionalGroupsSequence);

      DcmItem& content = AppendItem(perFrame, DCM_FrameContentSequence);
      Require(content.putAndInsertString(DCM_StackID, "1"), "Stack ID");
      Require(content.putAndInsertUint32(DCM_InStackPositionNumber, frame.inStackPosition),
              "In-Stack Position Number");
      const Uint32 dimensionIndexValues[2] = { 1, frame.inStackPosition };
      Require(content.putAndInsertUint32Array(DCM_DimensionIndexValues, dimensionIndexValues, 2),
              "Dimension Index Values");
      if (!frame.frameReferenceDateTime.empty())
      {
        Require(content.putAndInsertString(DCM_FrameReferenceDateTime, frame.frameReferenceDateTime.c_str()),
                "Frame Reference DateTime");
      }

      DcmItem& position = AppendItem(perFrame, DCM_PlanePositionSequence);
      Require(position.putAndInsertString(
                DCM_ImagePositionPatient,
                PositionString(this->zOffset + k * this->sliceSpacing).c_str()),
              "Image Position (Patient)");

      if (!this->rescaleInSharedGroup)
      {
        WritePixelValueTransformation(perFrame, frame.slope, frame.intercept);
      }
    }
  }
  else
  {
    Require(dataset.putAndInsertString(DCM_ImageOrientationPatient, "1\\0\\0\\0\\1\\0"),
            "Image Orientation (Patient)");
    Require(dataset.putAndInsertString(DCM_ImagePositionPatient, PositionString(this->zOffset).c_str()),
            "Image Position (Patient)");
    const std::string spacing = ToDicomString(this->sliceSpacing) + "\\" + ToDicomString(this->sliceSpacing);
    Require(dataset.putAndInsertString(DCM_PixelSpacing, spacing.c_str()), "Pixel Spacing");
    Require(dataset.putAndInsertString(DCM_SliceThickness, ToDicomString(this->sliceSpacing).c_str()),
            "Slice Thickness");
    Require(dataset.putAndInsertString(DCM_SpacingBetweenSlices, ToDicomString(this->sliceSpacing).c_str()),
            "Spacing Between Slices");
    Require(dataset.putAndInsertString(DCM_RescaleIntercept, ToDicomString(this->frames.front().intercept).c_str()),
            "Rescale Intercept");
    Require(dataset.putAndInsertString(DCM_RescaleSlope, ToDicomString(this->frames.front().slope).c_str()),
            "Rescale Slope");
  }

  if (this->duplicateAtTopLevel)
  {
    WritePixelValueTransformation(dataset, this->topLevelDuplicateSlope, 0.0);
  }

  std::vector<std::int16_t> buffer(static_cast<std::size_t>(this->rows) * this->columns * this->frames.size());
  auto target = buffer.begin();
  for (const auto& frame : this->frames)
  {
    target = std::fill_n(target, static_cast<std::size_t>(this->rows) * this->columns, frame.constant);
  }
  Require(dataset.putAndInsertUint16Array(DCM_PixelData,
                                          reinterpret_cast<const Uint16*>(buffer.data()),
                                          static_cast<unsigned long>(buffer.size())),
          "Pixel Data");

  const std::string path = directory + "/" + filename;
  Require(fileFormat.saveFile(path.c_str(), EXS_LittleEndianExplicit), "Cannot save " + path);

  return path;
}
