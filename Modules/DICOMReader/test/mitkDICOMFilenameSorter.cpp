/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMFilenameSorter.h"

#include <algorithm>

mitk::DICOMFilenameSorter
::DICOMFilenameSorter()
:DICOMDatasetSorter()
{
}

mitk::DICOMFilenameSorter
::~DICOMFilenameSorter()
{
}

mitk::DICOMFilenameSorter
::DICOMFilenameSorter(const DICOMFilenameSorter& other )
:DICOMDatasetSorter(other)
{
}

mitk::DICOMFilenameSorter&
mitk::DICOMFilenameSorter
::operator=(const DICOMFilenameSorter& other)
{
  if (this != &other)
  {
    DICOMDatasetSorter::operator=(other);
  }
  return *this;
}

bool
mitk::DICOMFilenameSorter
::operator==(const DICOMDatasetSorter& other) const
{
  return dynamic_cast<const DICOMFilenameSorter*>(&other) != nullptr;
}

mitk::DICOMTagList
mitk::DICOMFilenameSorter
::GetTagsOfInterest()
{
  return DICOMTagList();
}

void
mitk::DICOMFilenameSorter
::PrintConfiguration(std::ostream& os, const std::string& indent) const
{
  os << indent << "Sort alphabetically based on filenames" << std::endl;
}

bool
mitk::DICOMFilenameSorter::FilenameSort
::operator() (const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right)
{
  return left->GetFilenameIfAvailable().compare( right->GetFilenameIfAvailable() ) < 0 ;
}

void
mitk::DICOMFilenameSorter
::Sort()
{
  DICOMDatasetList output = GetInput(); // copy

  std::sort( output.begin(), output.end(), FilenameSort() );

  this->SetNumberOfOutputs(1);
  this->SetOutput(0, output);
}
