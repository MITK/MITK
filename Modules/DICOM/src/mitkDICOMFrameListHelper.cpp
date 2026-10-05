/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMFrameListHelper.h>

#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkFileSystem.h>

#include <algorithm>

std::vector<std::string> mitk::DistinctFilesInOrder(const DICOMImageFrameList& frames)
{
  std::vector<std::string> result;
  result.reserve(frames.size());

  for (const auto& frame : frames)
  {
    if (result.empty() || result.back() != frame->Filename)
    {
      result.push_back(frame->Filename);
    }
  }

  return result;
}

bool mitk::ContainsFile(const DICOMImageFrameList& frames, const std::string& file)
{
  const fs::path path(file);

  return std::any_of(frames.begin(), frames.end(), [&path](const DICOMImageFrameInfo::Pointer& frame)
    {
      return fs::path(frame->Filename) == path;
    });
}

mitk::DICOMTagCache::Pointer mitk::AnalyzeWithFrameModel(DICOMFileReader& reader, const StringList& files)
{
  reader.SetInputFiles(files);

  auto scanner = DICOMDCMTKTagScanner::New();
  scanner->AddTagPaths(reader.GetTagsOfInterest());
  scanner->SetReadFrameModel(true);
  scanner->SetInputFiles(files);
  scanner->Scan();

  auto cache = scanner->GetScanCache();
  reader.SetTagCache(cache);
  reader.AnalyzeInputFiles();

  return cache;
}
