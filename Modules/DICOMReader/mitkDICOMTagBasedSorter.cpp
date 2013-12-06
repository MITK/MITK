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

#include "mitkDICOMTagBasedSorter.h"

#include <algorithm>

mitk::DICOMTagBasedSorter
::DICOMTagBasedSorter()
:DICOMDatasetSorter()
{
}

mitk::DICOMTagBasedSorter
::~DICOMTagBasedSorter()
{
}

mitk::DICOMTagBasedSorter
::DICOMTagBasedSorter(const DICOMTagBasedSorter& other )
:DICOMDatasetSorter(other)
{
}

mitk::DICOMTagBasedSorter&
mitk::DICOMTagBasedSorter
::operator=(const DICOMTagBasedSorter& other)
{
  if (this != &other)
  {
    DICOMDatasetSorter::operator=(other);
  }
  return *this;
}

mitk::DICOMTagList
mitk::DICOMTagBasedSorter
::GetTagsOfInterest()
{
  DICOMTagList allTags = m_DistinguishingTags;
  allTags.insert( allTags.end(), m_SortCriteria.begin(), m_SortCriteria.end() ); // append

  // TODO sort, unique

  return allTags;
}

void
mitk::DICOMTagBasedSorter
::AddDistinguishingTag( const DICOMTag& tag )
{
  m_DistinguishingTags.push_back(tag);
}

void
mitk::DICOMTagBasedSorter
::AddSortCriterion( const DICOMTag& tag )
{
  m_SortCriteria.push_back(tag);
}

void
mitk::DICOMTagBasedSorter
::Sort()
{
  // 1. split
  this->SplitGroups();

  // 2. sort each group
  this->SortGroups();

  //std::sort( output.begin(), output.end(), FilenameSort() );
}

std::string
mitk::DICOMTagBasedSorter
::BuildGroupID( DICOMDatasetAccess* dataset )
{
  // just concatenate all tag values
  assert(dataset);
  std::stringstream groupID;
  groupID << "g";
  for (DICOMTagList::iterator tagIter = m_DistinguishingTags.begin();
       tagIter != m_DistinguishingTags.end();
       ++tagIter)
  {
    groupID << tagIter->first << tagIter->second; // make group/element part of the id to cover empty tags
    groupID << dataset->GetTagValueAsString(*tagIter);
  }
  // shorten ID?
  return groupID.str();
}

void
mitk::DICOMTagBasedSorter
::SplitGroups()
{
  DICOMDatasetList input = GetInput(); // copy

  typedef std::map<std::string, DICOMDatasetList> GroupIDToListType;
  GroupIDToListType listForGroupID;

  for (DICOMDatasetList::iterator dsIter = input.begin();
       dsIter != input.end();
       ++dsIter)
  {
    DICOMDatasetAccess* dataset = *dsIter;
    assert(dataset);

    std::string groupID = this->BuildGroupID( dataset );
    MITK_DEBUG << "Group ID for for " << dataset->GetFilenameIfAvailable() << ": " << groupID;
    listForGroupID[groupID].push_back(dataset);
  }

  this->SetNumberOfOutputs(listForGroupID.size());
  unsigned int outputIndex(0);
  for (GroupIDToListType::iterator groupIter = listForGroupID.begin();
       groupIter != listForGroupID.end();
       ++outputIndex, ++groupIter)
  {
    this->SetOutput(outputIndex, groupIter->second);
  }
}

void
mitk::DICOMTagBasedSorter
::SortGroups()
{
  // for each output
  //   sort by all configured tags, use secondary tags when equal or empty
  //   make configurable:
  //    - sorting order (ascending, descending)
  //    - sort numerically
  //    - ... ?
}
