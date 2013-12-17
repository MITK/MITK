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

mitk::DICOMImageBlockDescriptor
::DICOMImageBlockDescriptor()
:m_PixelsInterpolated(false)
,m_PixelSpacingInterpretation()
,m_PixelSpacing("")
,m_ImagerPixelSpacing("")
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
,m_PixelsInterpolated( other.m_PixelsInterpolated )
,m_PixelSpacingInterpretation( other.m_PixelSpacingInterpretation )
,m_PixelSpacing( other.m_PixelSpacing )
,m_ImagerPixelSpacing( other.m_ImagerPixelSpacing )
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
    m_PixelsInterpolated = other.m_PixelsInterpolated;
    m_PixelSpacingInterpretation = other.m_PixelSpacingInterpretation;
    m_PixelSpacing = other.m_PixelSpacing;
    m_ImagerPixelSpacing = other.m_ImagerPixelSpacing;
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
::SetPixelsInterpolated(bool pixelsAreInterpolated)
{
  if (m_PixelsInterpolated != pixelsAreInterpolated)
  {
    m_PixelsInterpolated = pixelsAreInterpolated;
  }
}

bool
mitk::DICOMImageBlockDescriptor
::GetPixelsInterpolated() const
{
  return m_PixelsInterpolated;
}


void
mitk::DICOMImageBlockDescriptor
::SetPixelSpacingInterpretation( PixelSpacingInterpretation interpretation )
{
  if (m_PixelSpacingInterpretation != interpretation)
  {
    m_PixelSpacingInterpretation = interpretation;
  }
}

mitk::PixelSpacingInterpretation
mitk::DICOMImageBlockDescriptor
::GetPixelSpacingInterpretation() const
{
  return m_PixelSpacingInterpretation;
}

void
mitk::DICOMImageBlockDescriptor
::SetPixelSpacingInformation(const std::string& pixelSpacing, const std::string& imagerPixelSpacing)
{
  m_PixelSpacing = pixelSpacing;
  m_ImagerPixelSpacing = imagerPixelSpacing;
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
  if (mitkImage && m_TagCache)
  {
    // first part: add some tags that describe individual slices


    // TODO make sure this is available from scanning!
    const DICOMTag tagSliceLocation(0x0020,0x1041);
    const DICOMTag tagInstanceNumber(0x0020,0x0013);
    const DICOMTag tagSOPInstanceNumber(0x0008,0x0018);

    unsigned int numberOfTimeSteps = mitkImage->GetDimension(3);
    MITK_INFO << "Properties for " << numberOfTimeSteps << " timesteps";
    unsigned int numberOfFramesPerTimestep = m_ImageFrameList.size() / numberOfTimeSteps;

    DICOMImageFrameList::const_iterator frameIter = m_ImageFrameList.begin();
    for (unsigned int timeStep = 0;
         timeStep < numberOfTimeSteps;
         ++timeStep)
    {
      std::string propertyKeySliceLocation = "dicom.image.0020.1041";
      std::string propertyKeyInstanceNumber = "dicom.image.0020.0013";
      std::string propertyKeySOPInstanceNumber = "dicom.image.0008.0018";

      StringLookupTable filesForSlices;
      StringLookupTable sliceLocationForSlices;
      StringLookupTable instanceNumberForSlices;
      StringLookupTable SOPInstanceNumberForSlices;

      for (unsigned int slice = 0;
           slice < numberOfFramesPerTimestep;
           ++slice, ++frameIter)
      {
        std::string sliceLocation = m_TagCache->GetTagValue( *frameIter, tagSliceLocation );
        sliceLocationForSlices.SetTableValue(slice, sliceLocation);

        std::string instanceNumber = m_TagCache->GetTagValue( *frameIter, tagInstanceNumber );
        instanceNumberForSlices.SetTableValue(slice, instanceNumber);

        std::string sopInstanceUID = m_TagCache->GetTagValue( *frameIter, tagSOPInstanceNumber );
        SOPInstanceNumberForSlices.SetTableValue(slice, sopInstanceUID);

        MITK_INFO << "Tag info for slice " << slice
                  << ": SL '" << sliceLocation
                  << "' IN '" << instanceNumber
                  << "' SOP instance UID '" << sopInstanceUID << "'";
      }

      if(timeStep != 0)
      {
        std::ostringstream postfix;
        postfix << ".t" << timeStep;

        propertyKeySliceLocation.append(postfix.str());
        propertyKeyInstanceNumber.append(postfix.str());
        propertyKeySOPInstanceNumber.append(postfix.str());
      }

      // add property or properties with proper names
      mitkImage->SetProperty( propertyKeySliceLocation.c_str(), StringLookupTableProperty::New( sliceLocationForSlices ) );
      mitkImage->SetProperty( propertyKeyInstanceNumber.c_str(),    StringLookupTableProperty::New( instanceNumberForSlices ) );
      mitkImage->SetProperty( propertyKeySOPInstanceNumber.c_str(), StringLookupTableProperty::New( SOPInstanceNumberForSlices ) );
    }


    // second part: add properties that describe the whole image block

    // TODO at this point here, simply provide all properties of DICOMImageBlockDescriptor as image properties
    // TODO copy imageblockdescriptor as properties
    /*
    mitkImage->SetProperty("dicomseriesreader.SOPClass", StringProperty::New(blockInfo.GetSOPClassUIDAsString()));
    mitkImage->SetProperty("dicomseriesreader.ReaderImplementationLevelString", StringProperty::New(ReaderImplementationLevelToString( blockInfo.GetReaderImplementationLevel() )));
    mitkImage->SetProperty("dicomseriesreader.ReaderImplementationLevel", GenericProperty<ReaderImplementationLevel>::New( blockInfo.GetReaderImplementationLevel() ));
    mitkImage->SetProperty("dicomseriesreader.PixelSpacingInterpretationString", StringProperty::New(PixelSpacingInterpretationToString( blockInfo.GetPixelSpacingType() )));
    mitkImage->SetProperty("dicomseriesreader.PixelSpacingInterpretation", GenericProperty<PixelSpacingInterpretation>::New(blockInfo.GetPixelSpacingType()));
    mitkImage->SetProperty("dicomseriesreader.MultiFrameImage", BoolProperty::New(blockInfo.IsMultiFrameImage()));
    mitkImage->SetProperty("dicomseriesreader.GantyTiltCorrected", BoolProperty::New(blockInfo.HasGantryTiltCorrected()));
    mitkImage->SetProperty("dicomseriesreader.3D+t", BoolProperty::New(blockInfo.HasMultipleTimePoints()));
    */


    // third part: get something from ImageIO. BUT this needs to be created elsewhere. or not at all!

  }
  return mitkImage;
}

void
mitk::DICOMImageBlockDescriptor
::SetTagCache(DICOMTagCache* privateCache)
{
  // TODO this must only be used during loading and never afterwards
  // TODO better: somehow make possible to have smartpointers here
  m_TagCache = privateCache;
}
