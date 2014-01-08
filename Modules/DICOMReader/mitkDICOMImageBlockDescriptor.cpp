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

#include "mitkDICOMImageBlockDescriptor.h"
#include "mitkStringProperty.h"
#include <gdcmUIDs.h>

mitk::DICOMImageBlockDescriptor
::DICOMImageBlockDescriptor()
:m_ReaderImplementationLevel(SOPClassUnknown)
,m_PixelSpacing("")
,m_ImagerPixelSpacing("")
,m_SOPClassUID("")
,m_PropertyList(PropertyList::New())
,m_TagCache(NULL)
{
}

mitk::DICOMImageBlockDescriptor
::~DICOMImageBlockDescriptor()
{
}

mitk::DICOMImageBlockDescriptor
::DICOMImageBlockDescriptor(const DICOMImageBlockDescriptor& other)
:m_ImageFrameList( other.m_ImageFrameList )
,m_MitkImage( other.m_MitkImage )
,m_SliceIsLoaded( other.m_SliceIsLoaded )
,m_ReaderImplementationLevel( other.m_ReaderImplementationLevel )
,m_PixelSpacing( other.m_PixelSpacing )
,m_ImagerPixelSpacing( other.m_ImagerPixelSpacing )
,m_SOPClassUID( other.m_SOPClassUID )
,m_TiltInformation( other.m_TiltInformation )
,m_PropertyList( other.m_PropertyList->Clone() )
,m_TagCache( other.m_TagCache )
{
  if (m_MitkImage)
  {
    m_MitkImage = m_MitkImage->Clone();
  }
}

mitk::DICOMImageBlockDescriptor&
mitk::DICOMImageBlockDescriptor
::operator=(const DICOMImageBlockDescriptor& other)
{
  if (this != &other)
  {
    m_ImageFrameList = other.m_ImageFrameList;
    m_MitkImage = other.m_MitkImage;
    m_SliceIsLoaded = other.m_SliceIsLoaded;
    m_ReaderImplementationLevel = other.m_ReaderImplementationLevel;
    m_PixelSpacing = other.m_PixelSpacing;
    m_ImagerPixelSpacing = other.m_ImagerPixelSpacing;
    m_SOPClassUID = other.m_SOPClassUID;
    m_TiltInformation = other.m_TiltInformation;

    if (other.m_PropertyList)
    {
      m_PropertyList = other.m_PropertyList->Clone();
    }

    if (other.m_MitkImage)
    {
      m_MitkImage = other.m_MitkImage->Clone();
    }

    m_TagCache = other.m_TagCache;
  }
  return *this;
}

void
mitk::DICOMImageBlockDescriptor
::SetTiltInformation(const GantryTiltInformation& info)
{
  m_TiltInformation = info;
}

const mitk::GantryTiltInformation
mitk::DICOMImageBlockDescriptor
::GetTiltInformation() const
{
  return m_TiltInformation;
}

void
mitk::DICOMImageBlockDescriptor
::SetImageFrameList(const DICOMImageFrameList& framelist)
{
  m_ImageFrameList = framelist;
  m_SliceIsLoaded.resize(framelist.size());
  m_SliceIsLoaded.assign(framelist.size(), false);

  this->UpdateImageDescribingProperties();
}

const mitk::DICOMImageFrameList&
mitk::DICOMImageBlockDescriptor
::GetImageFrameList() const
{
  return m_ImageFrameList;
}

void
mitk::DICOMImageBlockDescriptor
::SetMitkImage(Image::Pointer image)
{
  if (m_MitkImage != image)
  {
    m_MitkImage = this->DescribeImageWithProperties( this->FixupSpacing(image) );
  }
}

mitk::Image::Pointer
mitk::DICOMImageBlockDescriptor
::GetMitkImage() const
{
  return m_MitkImage;
}

mitk::Image::Pointer
mitk::DICOMImageBlockDescriptor
::FixupSpacing(Image* mitkImage)
{
  if (mitkImage)
  {
    Vector3D imageSpacing = mitkImage->GetGeometry()->GetSpacing();

    ScalarType desiredSpacingX = imageSpacing[0];
    ScalarType desiredSpacingY = imageSpacing[1];
    this->GetDesiredMITKImagePixelSpacing( desiredSpacingX, desiredSpacingY ); // prefer pixel spacing over imager pixel spacing

    MITK_DEBUG << "Loaded image with spacing " << imageSpacing[0] << ", " << imageSpacing[1];
    MITK_DEBUG << "Found correct spacing info " << desiredSpacingX << ", " << desiredSpacingY;

    imageSpacing[0] = desiredSpacingX;
    imageSpacing[1] = desiredSpacingY;
    mitkImage->GetGeometry()->SetSpacing( imageSpacing );
  }

  return mitkImage;
}
void
mitk::DICOMImageBlockDescriptor
::SetSliceIsLoaded(unsigned int index, bool isLoaded)
{
  if (index < m_SliceIsLoaded.size())
  {
    m_SliceIsLoaded[index] = isLoaded;
  }
  else
  {
    std::stringstream ss;
    ss << "Index " << index << " out of range (" << m_SliceIsLoaded.size() << " indices reserved)";
    throw std::invalid_argument( ss.str() );
  }
}

bool
mitk::DICOMImageBlockDescriptor
::IsSliceLoaded(unsigned int index) const
{
  if (index < m_SliceIsLoaded.size())
  {
    return m_SliceIsLoaded[index];
  }
  else
  {
    std::stringstream ss;
    ss << "Index " << index << " out of range (" << m_SliceIsLoaded.size() << " indices reserved)";
    throw std::invalid_argument( ss.str() );
  }
}

bool
mitk::DICOMImageBlockDescriptor
::AllSlicesAreLoaded() const
{
  bool allLoaded = true;
  for (BoolList::const_iterator iter = m_SliceIsLoaded.begin();
       iter != m_SliceIsLoaded.end();
       ++iter)
  {
    allLoaded &= *iter;
  }

  return allLoaded;
}

void
mitk::DICOMImageBlockDescriptor
::SetPixelSpacingTagValues(const std::string& pixelSpacing, const std::string& imagerPixelSpacing)
{
  m_PixelSpacing = pixelSpacing;
  m_ImagerPixelSpacing = imagerPixelSpacing;
}

/*
   PS defined      IPS defined     PS==IPS
        0               0                     --> UNKNOWN spacing, loader will invent
        0               1                     --> spacing as at detector surface
        1               0                     --> spacing as in patient
        1               1             0       --> detector surface spacing CORRECTED for geometrical magnifications: spacing as in patient
        1               1             1       --> detector surface spacing NOT corrected for geometrical magnifications: spacing as at detector
*/
mitk::PixelSpacingInterpretation
mitk::DICOMImageBlockDescriptor
::GetPixelSpacingInterpretation() const
{
  if (m_PixelSpacing.empty())
  {
    if (m_ImagerPixelSpacing.empty())
    {
      return SpacingUnknown;
    }
    else
    {
      return SpacingAtDetector;
    }
  }
  else // Pixel Spacing defined
  {
    if (m_ImagerPixelSpacing.empty())
    {
      return SpacingInPatient;
    }
    else if (m_PixelSpacing != m_ImagerPixelSpacing)
    {
      return SpacingInPatient;
    }
    else
    {
      return SpacingAtDetector;
    }
  }
}

void
mitk::DICOMImageBlockDescriptor
::GetDesiredMITKImagePixelSpacing( ScalarType& spacingX, ScalarType& spacingY) const
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

void
mitk::DICOMImageBlockDescriptor
::SetProperty(const std::string& key, BaseProperty* value)
{
  m_PropertyList->SetProperty(key, value);
}

mitk::BaseProperty*
mitk::DICOMImageBlockDescriptor
::GetProperty(const std::string& key) const
{
  return m_PropertyList->GetProperty(key);
}

std::string
mitk::DICOMImageBlockDescriptor
::GetPropertyAsString(const std::string& key) const
{
  mitk::BaseProperty::Pointer property = m_PropertyList->GetProperty(key);
  if (property.IsNotNull())
  {
    return property->GetValueAsString();
  }
  else
  {
    return std::string("");
  }
}

void
mitk::DICOMImageBlockDescriptor
::SetFlag(const std::string& key, bool value)
{
  m_PropertyList->ReplaceProperty(key, BoolProperty::New(value));
}

bool
mitk::DICOMImageBlockDescriptor
::GetFlag(const std::string& key, bool defaultValue) const
{
  BoolProperty::ConstPointer boolProp = dynamic_cast<BoolProperty*>( this->GetProperty(key) );
  if (boolProp.IsNotNull())
  {
    return boolProp->GetValue();
  }
  else
  {
    return defaultValue;
  }
}

void
mitk::DICOMImageBlockDescriptor
::SetIntProperty(const std::string& key, int value)
{
  m_PropertyList->ReplaceProperty(key, IntProperty::New(value));
}

int
mitk::DICOMImageBlockDescriptor
::GetIntProperty(const std::string& key, int defaultValue) const
{
  IntProperty::ConstPointer intProp = dynamic_cast<IntProperty*>( this->GetProperty(key) );
  if (intProp.IsNotNull())
  {
    return intProp->GetValue();
  }
  else
  {
    return defaultValue;
  }
}

mitk::Image::Pointer
mitk::DICOMImageBlockDescriptor
::DescribeImageWithProperties(Image* mitkImage)
{
  if (!mitkImage) return mitkImage;

  // first part: add some tags that describe individual slices
  // these propeties are defined at analysis time (see UpdateImageDescribingProperties())

  std::string propertyKeySliceLocation = "dicom.image.0020.1041";
  std::string propertyKeyInstanceNumber = "dicom.image.0020.0013";
  std::string propertyKeySOPInstanceUID = "dicom.image.0008.0018";

  mitkImage->SetProperty( propertyKeySliceLocation.c_str(), this->GetProperty("sliceLocationForSlices") );
  mitkImage->SetProperty( propertyKeyInstanceNumber.c_str(), this->GetProperty("instanceNumberForSlices") );
  mitkImage->SetProperty( propertyKeySOPInstanceUID.c_str(), this->GetProperty("SOPInstanceUIDForSlices") );


  // second part: add properties that describe the whole image block
  mitkImage->SetProperty("dicomseriesreader.SOPClassUID",
      StringProperty::New( m_SOPClassUID ) );

  mitkImage->SetProperty("dicomseriesreader.SOPClass",
      StringProperty::New( this->GetSOPClassUIDAsName() ));

  mitkImage->SetProperty("dicomseriesreader.PixelSpacingInterpretationString",
      StringProperty::New( PixelSpacingInterpretationToString( this->GetPixelSpacingInterpretation() )) );
  mitkImage->SetProperty("dicomseriesreader.PixelSpacingInterpretation",
      GenericProperty<PixelSpacingInterpretation>::New( this->GetPixelSpacingInterpretation() ));

  mitkImage->SetProperty("dicomseriesreader.ReaderImplementationLevelString",
      StringProperty::New( ReaderImplementationLevelToString( m_ReaderImplementationLevel ) ));
  mitkImage->SetProperty("dicomseriesreader.ReaderImplementationLevel",
      GenericProperty<ReaderImplementationLevel>::New( m_ReaderImplementationLevel ));

  mitkImage->SetProperty("dicomseriesreader.GantyTiltCorrected",
      BoolProperty::New( this->GetFlag("gantryTilt",false) ));

  mitkImage->SetProperty("dicomseriesreader.3D+t",
      BoolProperty::New( this->GetFlag("3D+t",false) ));

  // third part: get something from ImageIO. BUT this needs to be created elsewhere. or not at all!

  return mitkImage;
}

void
mitk::DICOMImageBlockDescriptor
::SetReaderImplementationLevel(const ReaderImplementationLevel& level)
{
  m_ReaderImplementationLevel = level;
}

mitk::ReaderImplementationLevel
mitk::DICOMImageBlockDescriptor
::GetReaderImplementationLevel() const
{
  return m_ReaderImplementationLevel;
}

void
mitk::DICOMImageBlockDescriptor
::SetSOPClassUID(const std::string& uid)
{
  m_SOPClassUID = uid;
}

std::string
mitk::DICOMImageBlockDescriptor
::GetSOPClassUID() const
{
  return m_SOPClassUID;
}

std::string
mitk::DICOMImageBlockDescriptor
::GetSOPClassUIDAsName() const
{
  gdcm::UIDs uidKnowledge;
  uidKnowledge.SetFromUID( m_SOPClassUID.c_str() );

  const char* name = uidKnowledge.GetName();
  if (name)
  {
    return std::string(name);
  }
  else
  {
    return std::string("");
  }
}


void
mitk::DICOMImageBlockDescriptor
::SetTagCache(DICOMTagCache* privateCache)
{
  // TODO this must only be used during loading and never afterwards
  // TODO better: somehow make possible to have smartpointers here
  m_TagCache = privateCache;
}

#define printPropertyRange(label, property_name) \
{ \
  std::string first = this->GetPropertyAsString( #property_name "First"); \
  std::string last = this->GetPropertyAsString( #property_name "Last"); \
  if (first == last) \
  { \
    os << "  " label ": '" << first << "'" << std::endl; \
  } \
  else \
  { \
    os << "  " label ": '" << first << "' - '" << last << "'" << std::endl; \
  } \
}

#define printProperty(label, property_name) \
{ \
  std::string first = this->GetPropertyAsString( #property_name ); \
  os << "  " label ": '" << first << "'" << std::endl; \
}

#define printBool(label, commands) \
{ \
  os << "  " label ": '" << (commands?"yes":"no") << "'" << std::endl; \
}


void
mitk::DICOMImageBlockDescriptor
::Print(std::ostream& os, bool filenameDetails) const
{
  os << "  Number of Frames: '" << m_ImageFrameList.size() << "'" << std::endl;
  os << "  SOP class: '" << this->GetSOPClassUIDAsName() << "'" << std::endl;

  printProperty("Modality", modality);
  printProperty("Sequence Name", sequenceName);

  printPropertyRange("Slice Location", sliceLocation);
  printPropertyRange("Acquisition Number", acquisitionNumber);
  printPropertyRange("Instance Number", instanceNumber);
  printPropertyRange("Image Position", imagePositionPatient);
  printProperty("Image Orientation", orientation);

  os << "  Pixel spacing interpretation: '" << PixelSpacingInterpretationToString(this->GetPixelSpacingInterpretation()) << "'" << std::endl;
  printBool("Gantry Tilt", this->GetFlag("gantryTilt",false))
  printBool("3D+t", this->GetFlag("3D+t",false))

  os << "  MITK image loaded: '" << (this->GetMitkImage().IsNotNull() ? "yes" : "no") << "'" << std::endl;
  if (filenameDetails)
  {
    for (DICOMImageFrameList::const_iterator frameIter = m_ImageFrameList.begin();
        frameIter != m_ImageFrameList.end();
        ++frameIter)
    {
      os << "  File " << (*frameIter)->Filename;
      if ((*frameIter)->FrameNo > 0)
      {
        os << ", " << (*frameIter)->FrameNo;
      }
      os << std::endl;
    }
  }
}

#define storeTagValueToProperty(tag_name, tag_g, tag_e) \
{ \
  const DICOMTag t(tag_g, tag_e); \
  std::string tagValue = m_TagCache->GetTagValue( firstFrame, t ); \
  this->SetProperty(#tag_name, StringProperty::New( tagValue ) ); \
}

#define storeTagValueRangeToProperty(tag_name, tag_g, tag_e) \
{ \
  const DICOMTag t(tag_g, tag_e); \
  std::string tagValueFirst = m_TagCache->GetTagValue( firstFrame, t ); \
  std::string tagValueLast = m_TagCache->GetTagValue( lastFrame, t ); \
  this->SetProperty(#tag_name "First", StringProperty::New( tagValueFirst ) ); \
  this->SetProperty(#tag_name "Last", StringProperty::New( tagValueLast ) ); \
}


void
mitk::DICOMImageBlockDescriptor
::UpdateImageDescribingProperties()
{
  if (m_TagCache && !m_ImageFrameList.empty())
  {
    DICOMImageFrameInfo::Pointer firstFrame = m_ImageFrameList.front();;
    DICOMImageFrameInfo::Pointer lastFrame = m_ImageFrameList.back();;

    // see macros above
    storeTagValueToProperty(modality,0x0008,0x0060)
    storeTagValueToProperty(sequenceName,0x0018,0x0024)
    storeTagValueToProperty(orientation,0x0020,0x0037)

    storeTagValueRangeToProperty(sliceLocation,0x0020,0x1041)
    storeTagValueRangeToProperty(acquisitionNumber,0x0020,0x0012)
    storeTagValueRangeToProperty(instanceNumber,0x0020,0x0013)
    storeTagValueRangeToProperty(imagePositionPatient,0x0020,0x0032)

    // some per-image attributes
    // frames are just numbered starting from 0. timestep 1 (the second time-step) has frames starting at (number-of-frames-per-timestep)
    std::string propertyKeySliceLocation = "dicom.image.0020.1041";
    std::string propertyKeyInstanceNumber = "dicom.image.0020.0013";
    std::string propertyKeySOPInstanceNumber = "dicom.image.0008.0018";

    StringLookupTable sliceLocationForSlices;
    StringLookupTable instanceNumberForSlices;
    StringLookupTable SOPInstanceUIDForSlices;

    const DICOMTag tagSliceLocation(0x0020,0x1041);
    const DICOMTag tagInstanceNumber(0x0020,0x0013);
    const DICOMTag tagSOPInstanceNumber(0x0008,0x0018);

    unsigned int slice(0);
    for (DICOMImageFrameList::const_iterator frameIter = m_ImageFrameList.begin();
         frameIter != m_ImageFrameList.end();
         ++slice, ++frameIter)
    {
      std::string sliceLocation = m_TagCache->GetTagValue( *frameIter, tagSliceLocation );
      sliceLocationForSlices.SetTableValue(slice, sliceLocation);

      std::string instanceNumber = m_TagCache->GetTagValue( *frameIter, tagInstanceNumber );
      instanceNumberForSlices.SetTableValue(slice, instanceNumber);

      std::string sopInstanceUID = m_TagCache->GetTagValue( *frameIter, tagSOPInstanceNumber );
      SOPInstanceUIDForSlices.SetTableValue(slice, sopInstanceUID);

      MITK_DEBUG << "Tag info for slice " << slice
                 << ": SL '" << sliceLocation
                 << "' IN '" << instanceNumber
                 << "' SOP instance UID '" << sopInstanceUID << "'";

      // add property or properties with proper names
      this->SetProperty( "sliceLocationForSlices", StringLookupTableProperty::New( sliceLocationForSlices ) );
      this->SetProperty( "instanceNumberForSlices",    StringLookupTableProperty::New( instanceNumberForSlices ) );
      this->SetProperty( "SOPInstanceUIDForSlices", StringLookupTableProperty::New( SOPInstanceUIDForSlices ) );
    }
  }
}
