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

#include "mitkDICOMGenericTagCache.h"
#include "mitkDICOMEnums.h"
#include "mitkDICOMGenericImageFrameInfo.h"

mitk::DICOMGenericTagCache::DICOMGenericTagCache()
{
}

mitk::DICOMGenericTagCache::~DICOMGenericTagCache()
{
}

mitk::DICOMDatasetFinding mitk::DICOMGenericTagCache::GetTagValue( DICOMImageFrameInfo* frame, const DICOMTag& tag ) const
{
  auto findings =  GetTagValue(frame, DICOMTagPath(tag));

  DICOMDatasetFinding result;
  if (!findings.empty())
  {
    result = findings.front();
  }
  return result;
}

mitk::DICOMDatasetAccess::FindingsListType
mitk::DICOMGenericTagCache::GetTagValue(DICOMImageFrameInfo* frame, const DICOMTagPath& path) const
{
  FindingsListType result;

  for (auto info : m_ScanResult)
  {
    if (info == frame)
    {
      result = info->GetTagValueAsString(path);
    }
  }
  return result;
}

mitk::DICOMDatasetAccessingImageFrameList mitk::DICOMGenericTagCache::GetFrameInfoList() const
{
  return m_ScanResult;
}

void
mitk::DICOMGenericTagCache::AddFrameInfo(DICOMDatasetAccessingImageFrameInfo* info)
{
  m_ScanResult.push_back(info);
};

void
mitk::DICOMGenericTagCache::Reset()
{
  m_ScanResult.clear();
};
