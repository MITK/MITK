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

#include "mitkDICOMDatasetAccessingImageFrameInfo.h"

mitk::DICOMDatasetAccessingImageFrameInfo
::DICOMDatasetAccessingImageFrameInfo(const std::string& filename, unsigned int frameNo)
:DICOMImageFrameInfo(filename, frameNo)
{
}

mitk::DICOMDatasetAccessingImageFrameInfo
::~DICOMDatasetAccessingImageFrameInfo()
{
}

mitk::DICOMImageFrameList
mitk::ConvertToDICOMImageFrameList(const DICOMDatasetAccessingImageFrameList& input)
{
  DICOMImageFrameList output;
  output.reserve(input.size());

  for (auto& inputIter : input)
  {
    DICOMImageFrameInfo* fi = inputIter.GetPointer();
    assert(fi);
    output.push_back(fi);
  }

  return output;
}

mitk::DICOMDatasetList
mitk::ConvertToDICOMDatasetList(const DICOMDatasetAccessingImageFrameList& input)
{
  DICOMDatasetList output;
  output.reserve(input.size());

  for (auto& inputIter : input)
  {
    DICOMDatasetAccess* da = inputIter.GetPointer();
    assert(da);
    output.push_back(da);
  }

  return output;
}

mitk::DICOMDatasetAccessingImageFrameList
mitk::ConvertToDICOMDatasetAccessingImageFrameList(const DICOMDatasetList& input)
{
  DICOMDatasetAccessingImageFrameList output;
  output.reserve(input.size());

  for (auto& inputIter : input)
  {
    auto* afi = dynamic_cast<DICOMDatasetAccessingImageFrameInfo*>(inputIter);
    assert(afi);
    output.push_back(afi);
  }

  return output;
}
