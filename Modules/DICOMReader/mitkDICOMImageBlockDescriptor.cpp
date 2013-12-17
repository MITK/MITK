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
    m_MitkImage = image;
  }
}

mitk::Image::Pointer
mitk::DICOMImageBlockDescriptor
::GetMitkImage() const
{
  return m_MitkImage;
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
