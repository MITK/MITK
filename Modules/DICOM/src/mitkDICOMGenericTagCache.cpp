/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMGenericTagCache.h>
#include <mitkDICOMEnums.h>
#include <mitkDICOMGenericImageFrameInfo.h>

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
  // The passed info decides which view answers. Resolving by filename and
  // FrameNo instead could not tell a file-level info from frame-scoped frame 0,
  // which is exactly the distinction the frame model rests on.
  if (const auto* access = dynamic_cast<const DICOMDatasetAccess*>(frame))
  {
    return access->GetTagValueAsString(path);
  }

  return {};
}

mitk::DICOMDatasetAccessingImageFrameList mitk::DICOMGenericTagCache::GetFrameInfoList() const
{
  return m_ScanResult;
}

const mitk::DICOMGenericImageFrameInfo* mitk::DICOMGenericTagCache::FindFile(const std::string& filename) const
{
  const auto finding = m_ByFilename.find(filename);

  return m_ByFilename.cend() == finding ? nullptr : finding->second;
}

mitk::DICOMFrameLayout mitk::DICOMGenericTagCache::GetFrameLayout(const DICOMImageFrameInfo* frame) const
{
  if (nullptr == frame)
  {
    return DICOMFrameLayout();
  }

  const auto* file = this->FindFile(frame->Filename);

  return nullptr != file ? file->GetFrameLayout() : DICOMFrameLayout();
}

bool mitk::DICOMGenericTagCache::HasAnyFrameModel() const
{
  return m_HasAnyFrameModel;
}

mitk::DICOMDatasetAccessingImageFrameInfo::Pointer
mitk::DICOMGenericTagCache::GetFrameInfo(const std::string& filename, unsigned int frameNo) const
{
  const auto* file = this->FindFile(filename);
  if (nullptr == file || !file->GetFrameLayout().HasFrameModel() || frameNo >= file->GetFrameLayout().frameCount)
  {
    return nullptr;
  }

  auto& frames = m_FrameScoped[filename];
  if (frames.empty())
  {
    frames.resize(file->GetFrameLayout().frameCount);
  }

  if (frames[frameNo].IsNull())
  {
    frames[frameNo] = DICOMGenericImageFrameInfo::NewFrameScoped(filename, frameNo, file->GetStore()).GetPointer();
  }

  return frames[frameNo];
}

void
mitk::DICOMGenericTagCache::AddFrameInfo(DICOMDatasetAccessingImageFrameInfo* info)
{
  m_ScanResult.push_back(info);

  if (auto* generic = dynamic_cast<DICOMGenericImageFrameInfo*>(info))
  {
    m_ByFilename[generic->Filename] = generic;
    m_HasAnyFrameModel |= generic->GetFrameLayout().HasFrameModel();
  }
};

void
mitk::DICOMGenericTagCache::Reset()
{
  m_ScanResult.clear();
  m_ByFilename.clear();
  m_FrameScoped.clear();
  m_HasAnyFrameModel = false;
};
