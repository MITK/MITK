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
  return dynamic_cast<const DICOMFilenameSorter*>(&other) != NULL;
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
