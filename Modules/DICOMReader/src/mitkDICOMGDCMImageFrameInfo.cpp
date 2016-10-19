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
:DICOMDatasetAccessingImageFrameInfo(filename, frameNo)
,m_TagForValue()
{
}

mitk::DICOMGDCMImageFrameInfo
::DICOMGDCMImageFrameInfo(const DICOMImageFrameInfo::Pointer& frameinfo)
:DICOMDatasetAccessingImageFrameInfo(frameinfo->Filename, frameinfo->FrameNo)
,m_TagForValue()
{
}

mitk::DICOMGDCMImageFrameInfo
::DICOMGDCMImageFrameInfo(const DICOMImageFrameInfo::Pointer& frameinfo, gdcm::Scanner::TagToValue const& tagToValueMapping)
:DICOMDatasetAccessingImageFrameInfo(frameinfo->Filename, frameinfo->FrameNo)
,m_TagForValue(tagToValueMapping)
{
}

mitk::DICOMGDCMImageFrameInfo::
~DICOMGDCMImageFrameInfo()
{
}

mitk::DICOMDatasetFinding
mitk::DICOMGDCMImageFrameInfo
::GetTagValueAsString(const DICOMTag& tag) const
{
  const auto mappedValue = m_TagForValue.find( gdcm::Tag(tag.GetGroup(), tag.GetElement()) );
  DICOMDatasetFinding result;

  if (mappedValue != m_TagForValue.cend())
  {
    result.isValid = true;

    if (mappedValue->second != nullptr)
    {
      std::string s(mappedValue->second);
      try
      {
        result.value = s.erase(s.find_last_not_of(" \n\r\t")+1);
      }
      catch(...)
      {
        result.value = s;
      }
    }
    else
    {
      result.value = "";
    }
  }
  else
  {
    const DICOMTag tagImagePositionPatient = DICOMTag(0x0020,0x0032); // Image Position (Patient)
    const DICOMTag    tagImageOrientation = DICOMTag(0x0020, 0x0037); // Image Orientation

    if (tag == tagImagePositionPatient)
    {
      result.isValid = true;
      result.value = std::string("0\\0\\0");
    }
    else if (tag == tagImageOrientation)
    {
      result.isValid = true;
      result.value = std::string("1\\0\\0\\0\\1\\0");
    }
    else
    {
      result.isValid = false;
      result.value = "";
    }
  }
  return result;
}

mitk::DICOMDatasetAccess::FindingsListType
mitk::DICOMGDCMImageFrameInfo::GetTagValueAsString(const DICOMTagPath& path) const
{
  FindingsListType result;
  if (path.Size() == 1 && path.IsExplicit())
  {
    result.push_back(this->GetTagValueAsString(path.GetFirstNode().tag));
  }
  return result;
}

std::string
mitk::DICOMGDCMImageFrameInfo
::GetFilenameIfAvailable() const
{
  return this->Filename;
}
