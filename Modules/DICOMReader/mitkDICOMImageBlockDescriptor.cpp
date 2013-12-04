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
{
}

mitk::DICOMImageBlockDescriptor
::~DICOMImageBlockDescriptor()
{
}

mitk::DICOMImageBlockDescriptor
::DICOMImageBlockDescriptor(const DICOMImageBlockDescriptor& other)
:m_Filenames( other.m_Filenames )
,m_MitkImage( other.m_MitkImage )
,m_SliceIsLoaded( other.m_SliceIsLoaded )
,m_PixelsInterpolated( other.m_PixelsInterpolated )
,m_PixelSpacingInterpretation( other.m_PixelSpacingInterpretation )
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
    m_Filenames = other.m_Filenames;
    m_MitkImage = other.m_MitkImage;
    m_SliceIsLoaded = other.m_SliceIsLoaded;
    m_PixelsInterpolated = other.m_PixelsInterpolated;
    m_PixelSpacingInterpretation = other.m_PixelSpacingInterpretation;

    if (m_MitkImage)
    {
      m_MitkImage = m_MitkImage->Clone();
    }
  }
  return *this;
}

void
mitk::DICOMImageBlockDescriptor
::SetFilenames(StringList filenames)
{
  m_Filenames = filenames;
  m_SliceIsLoaded.resize(filenames.size());
  m_SliceIsLoaded.assign(filenames.size(), false);
}

const mitk::StringList&
mitk::DICOMImageBlockDescriptor
::GetFilenames() const
{
  return m_Filenames;
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
