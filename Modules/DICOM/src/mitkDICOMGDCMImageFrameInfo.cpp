/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
