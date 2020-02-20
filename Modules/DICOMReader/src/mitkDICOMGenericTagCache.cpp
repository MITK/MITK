/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
