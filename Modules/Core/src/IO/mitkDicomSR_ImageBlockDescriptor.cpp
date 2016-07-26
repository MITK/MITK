/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkDicomSeriesReader.h>

#include <gdcmUIDs.h>

namespace mitk
{

DicomSeriesReader::ImageBlockDescriptor::ImageBlockDescriptor()
:m_HasGantryTiltCorrected(false)
,m_HasMultipleTimePoints(false)
,m_IsMultiFrameImage(false)
{
}

DicomSeriesReader::ImageBlockDescriptor::~ImageBlockDescriptor()
{
  // nothing
}

DicomSeriesReader::ImageBlockDescriptor::ImageBlockDescriptor(const StringContainer& files)
:m_HasGantryTiltCorrected(false)
,m_HasMultipleTimePoints(false)
,m_IsMultiFrameImage(false)
{
  m_Filenames = files;
}


void DicomSeriesReader::ImageBlockDescriptor::AddFile(const std::string& filename)
{
  m_Filenames.push_back( filename );
}

void DicomSeriesReader::ImageBlockDescriptor::AddFiles(const StringContainer& files)
{
  m_Filenames.insert( m_Filenames.end(), files.begin(), files.end() );
}

DicomSeriesReader::StringContainer DicomSeriesReader::ImageBlockDescriptor::GetFilenames() const
{
  return m_Filenames;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetImageBlockUID() const
{
  return m_ImageBlockUID;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetSeriesInstanceUID() const
{
  return m_SeriesInstanceUID;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetModality() const
{
  return m_Modality;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetSOPClassUIDAsString() const
{
  gdcm::UIDs uidKnowledge;
  uidKnowledge.SetFromUID( m_SOPClassUID.c_str() );
  return uidKnowledge.GetName();
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetSOPClassUID() const
{
  return m_SOPClassUID;
}

bool DicomSeriesReader::ImageBlockDescriptor::IsMultiFrameImage() const
{
  return m_IsMultiFrameImage;
}

DicomSeriesReader::ReaderImplementationLevel DicomSeriesReader::ImageBlockDescriptor::GetReaderImplementationLevel() const
{
  if ( this->IsMultiFrameImage() )
    return ReaderImplementationLevel_Unsupported;

  gdcm::UIDs uidKnowledge;
  uidKnowledge.SetFromUID( m_SOPClassUID.c_str() );

  gdcm::UIDs::TSType uid = uidKnowledge;

  switch (uid)
  {
    case gdcm::UIDs::CTImageStorage:
    case gdcm::UIDs::MRImageStorage:
    case gdcm::UIDs::PositronEmissionTomographyImageStorage:
    case gdcm::UIDs::ComputedRadiographyImageStorage:
    case gdcm::UIDs::DigitalXRayImageStorageForPresentation:
    case gdcm::UIDs::DigitalXRayImageStorageForProcessing:
      return ReaderImplementationLevel_Supported;

    case gdcm::UIDs::NuclearMedicineImageStorage:
      return ReaderImplementationLevel_PartlySupported;

    case gdcm::UIDs::SecondaryCaptureImageStorage:
      return ReaderImplementationLevel_Implemented;

    default:
      return ReaderImplementationLevel_Unsupported;
  }
}

bool DicomSeriesReader::ImageBlockDescriptor::HasGantryTiltCorrected() const
{
  return m_HasGantryTiltCorrected;
}

/*
   PS defined      IPS defined     PS==IPS
        0               0                     --> UNKNOWN spacing, loader will invent
        0               1                     --> spacing as at detector surface
        1               0                     --> spacing as in patient
        1               1             0       --> detector surface spacing CORRECTED for geometrical magnifications: spacing as in patient
        1               1             1       --> detector surface spacing NOT corrected for geometrical magnifications: spacing as at detector
*/
DicomSeriesReader::PixelSpacingInterpretation DicomSeriesReader::ImageBlockDescriptor::GetPixelSpacingType() const
{
  if (m_PixelSpacing.empty())
  {
    if (m_ImagerPixelSpacing.empty())
    {
      return PixelSpacingInterpretation_SpacingUnknown;
    }
    else
    {
      return PixelSpacingInterpretation_SpacingAtDetector;
    }
  }
  else // Pixel Spacing defined
  {
    if (m_ImagerPixelSpacing.empty())
    {
      return PixelSpacingInterpretation_SpacingInPatient;
    }
    else if (m_PixelSpacing != m_ImagerPixelSpacing)
    {
      return PixelSpacingInterpretation_SpacingInPatient;
    }
    else
    {
      return PixelSpacingInterpretation_SpacingAtDetector;
    }
  }
}

bool DicomSeriesReader::ImageBlockDescriptor::PixelSpacingRelatesToPatient() const
{
  return  GetPixelSpacingType() == PixelSpacingInterpretation_SpacingInPatient;
}

bool DicomSeriesReader::ImageBlockDescriptor::PixelSpacingRelatesToDetector() const
{
  return  GetPixelSpacingType() == PixelSpacingInterpretation_SpacingAtDetector;
}

bool DicomSeriesReader::ImageBlockDescriptor::PixelSpacingIsUnknown() const
{
  return GetPixelSpacingType() == PixelSpacingInterpretation_SpacingUnknown;
}

void DicomSeriesReader::ImageBlockDescriptor::SetPixelSpacingInformation(const std::string& pixelSpacing, const std::string& imagerPixelSpacing)
{
  m_PixelSpacing = pixelSpacing;
  m_ImagerPixelSpacing = imagerPixelSpacing;
}

void DicomSeriesReader::ImageBlockDescriptor::GetDesiredMITKImagePixelSpacing( ScalarType& spacingX, ScalarType& spacingY) const
{
  // preference for "in patient" pixel spacing
  if ( !DICOMStringToSpacing( m_PixelSpacing, spacingX, spacingY ) )
  {
    // fallback to "on detector" spacing
    if ( !DICOMStringToSpacing( m_ImagerPixelSpacing, spacingX, spacingY ) )
    {
      // last resort: invent something
      spacingX = spacingY = 1.0;
    }
  }
}

bool DicomSeriesReader::ImageBlockDescriptor::HasMultipleTimePoints() const
{
  return m_HasMultipleTimePoints;
}

void DicomSeriesReader::ImageBlockDescriptor::SetImageBlockUID(const std::string& uid)
{
  m_ImageBlockUID = uid;
}

void DicomSeriesReader::ImageBlockDescriptor::SetSeriesInstanceUID(const std::string& uid)
{
  m_SeriesInstanceUID = uid;
}

void DicomSeriesReader::ImageBlockDescriptor::SetModality(const std::string& modality)
{
  m_Modality = modality;
}

void DicomSeriesReader::ImageBlockDescriptor::SetNumberOfFrames(const std::string& numberOfFrames)
{
  m_IsMultiFrameImage = !numberOfFrames.empty();
}

void DicomSeriesReader::ImageBlockDescriptor::SetSOPClassUID(const std::string& sopClassUID)
{
  m_SOPClassUID = sopClassUID;
}


void DicomSeriesReader::ImageBlockDescriptor::SetHasGantryTiltCorrected(bool on)
{
  m_HasGantryTiltCorrected = on;
}

void DicomSeriesReader::ImageBlockDescriptor::SetHasMultipleTimePoints(bool on)
{
  m_HasMultipleTimePoints = on;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetOrientation() const
{
  return m_Orientation;
}

void DicomSeriesReader::ImageBlockDescriptor::SetOrientation(std::string orientation)
{
  m_Orientation = orientation;
}

} // end namespace mitk
