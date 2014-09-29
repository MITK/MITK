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

#include "mitkDICOMGDCMImageFrameInfo.h"

mitk::DICOMGDCMImageFrameInfo
::DICOMGDCMImageFrameInfo(const std::string& filename, unsigned int frameNo)
:itk::LightObject()
,m_FrameInfo( DICOMImageFrameInfo::New(filename, frameNo) )
,m_TagForValue()
{
}

mitk::DICOMGDCMImageFrameInfo
::DICOMGDCMImageFrameInfo(DICOMImageFrameInfo::Pointer frameinfo)
:itk::LightObject()
,m_FrameInfo(frameinfo)
,m_TagForValue()
{
}

mitk::DICOMGDCMImageFrameInfo
::DICOMGDCMImageFrameInfo(DICOMImageFrameInfo::Pointer frameinfo, gdcm::Scanner::TagToValue const& tagToValueMapping)
:itk::LightObject()
,m_FrameInfo(frameinfo)
,m_TagForValue(tagToValueMapping)
{
}

mitk::DICOMGDCMImageFrameInfo::
~DICOMGDCMImageFrameInfo()
{
}

std::string
mitk::DICOMGDCMImageFrameInfo
::GetTagValueAsString(const DICOMTag& tag) const
{
  gdcm::Scanner::TagToValue::const_iterator mappedValue = m_TagForValue.find( gdcm::Tag(tag.GetGroup(), tag.GetElement()) );

  if (mappedValue != m_TagForValue.end())
  {
    if (mappedValue->second != NULL)
    {
      std::string s(mappedValue->second);
      try
      {
        return s.erase(s.find_last_not_of(" \n\r\t")+1);
      }
      catch(...)
      {
        return s;
      }
    }
    else
    {
      return std::string("");
    }
  }
  else
  {
    const DICOMTag tagImagePositionPatient = DICOMTag(0x0020,0x0032); // Image Position (Patient)
    const DICOMTag    tagImageOrientation = DICOMTag(0x0020, 0x0037); // Image Orientation

    if (tag == tagImagePositionPatient)
    {
      return std::string("0\\0\\0");
    }
    else if (tag == tagImageOrientation)
    {
      return std::string("1\\0\\0\\0\\1\\0");
    }
    else
    {
      return std::string("");
    }
  }
}

std::string
mitk::DICOMGDCMImageFrameInfo
::GetFilenameIfAvailable() const
{
  if (m_FrameInfo.IsNotNull())
  {
    return m_FrameInfo->Filename;
  }
  else
  {
    return std::string("");
  }
}

mitk::DICOMImageFrameInfo::Pointer
mitk::DICOMGDCMImageFrameInfo
::GetFrameInfo() const
{
  return m_FrameInfo;
}

void
  mitk::DICOMGDCMImageFrameInfo
::SetFrameInfo(DICOMImageFrameInfo::Pointer frameinfo)
{
  m_FrameInfo = frameinfo;
}
