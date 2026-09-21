/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMFrameListHelper.h>

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
