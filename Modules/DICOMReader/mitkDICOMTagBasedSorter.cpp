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

  DICOMTagList sortingRelevantTags = m_SortCriterion->GetAllTagsOfInterest();
  allTags.insert( allTags.end(), sortingRelevantTags.begin(), sortingRelevantTags.end() ); // append

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
::SetSortCriterion( DICOMSortCriterion::ConstPointer criterion )
{
  m_SortCriterion = criterion;
}

void
mitk::DICOMTagBasedSorter
::Sort()
{
  // 1. split
  // 2. sort each group
  GroupIDToListType groups = this->SplitInputGroups();
  GroupIDToListType& sortedGroups = this->SortGroups( groups );

  // 3. define output
  this->SetNumberOfOutputs(sortedGroups.size());
  unsigned int outputIndex(0);
  for (GroupIDToListType::iterator groupIter = sortedGroups.begin();
       groupIter != sortedGroups.end();
       ++outputIndex, ++groupIter)
  {
    this->SetOutput(outputIndex, groupIter->second);
  }
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
    groupID << tagIter->GetGroup() << tagIter->GetElement(); // make group/element part of the id to cover empty tags
    groupID << dataset->GetTagValueAsString(*tagIter);
  }
  // shorten ID?
  return groupID.str();
}

mitk::DICOMTagBasedSorter::GroupIDToListType
mitk::DICOMTagBasedSorter
::SplitInputGroups()
{
  DICOMDatasetList input = GetInput(); // copy

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

  return listForGroupID;
}

mitk::DICOMTagBasedSorter::GroupIDToListType&
mitk::DICOMTagBasedSorter
::SortGroups(GroupIDToListType& groups)
{
  if (m_SortCriterion.IsNotNull())
  {
    // for each output
    //   sort by all configured tags, use secondary tags when equal or empty
    //   make configurable:
    //    - sorting order (ascending, descending)
    //    - sort numerically
    //    - ... ?
    for (GroupIDToListType::iterator gIter = groups.begin();
         gIter != groups.end();
         ++gIter)
    {
      DICOMDatasetList& dsList = gIter->second;
      std::sort( dsList.begin(), dsList.end(), ParameterizedDatasetSort( m_SortCriterion ) );
    }
  }

  return groups;
}

mitk::DICOMTagBasedSorter::ParameterizedDatasetSort
::ParameterizedDatasetSort(DICOMSortCriterion::ConstPointer criterion)
:m_SortCriterion(criterion)
{
}

bool
mitk::DICOMTagBasedSorter::ParameterizedDatasetSort
::operator() (const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right)
{
  assert(left);
  assert(right);
  assert(m_SortCriterion.IsNotNull());

  return m_SortCriterion->IsLeftBeforeRight(left, right);
}
