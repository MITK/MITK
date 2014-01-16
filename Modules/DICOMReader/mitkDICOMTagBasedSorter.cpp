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

mitk::DICOMTagBasedSorter::CutDecimalPlaces
::CutDecimalPlaces(unsigned int precision)
:m_Precision(precision)
{
}

std::string
mitk::DICOMTagBasedSorter::CutDecimalPlaces
::operator()(const std::string& input) const
{
  // be a bit tolerant for tags such as image orientation orienatation, let only the first few digits matter (http://bugs.mitk.org/show_bug.cgi?id=12263)
  // iterate all fields, convert each to a number, cut this number as configured, then return a concatenated string with all cut-off numbers
  static std::ostringstream resultString;
  resultString.str(std::string());
  resultString.clear();
  resultString.setf(std::ios::fixed, std::ios::floatfield);
  resultString.precision(m_Precision);

  static std::stringstream ss(input);
  ss.str(input);
  ss.clear();
  static std::string item;
  while (std::getline(ss, item, '\\'))
  {
    static std::istringstream converter(item);
    converter.str(item);
    converter.clear();
    static double number(0);
    if (converter >> number && converter.eof())
    {
      // converted to double
      resultString << number;
    }
    else
    {
      // did not convert to double
      resultString << item; // just paste the unmodified string
    }

    if (!ss.eof())
    {
      resultString << "\\";
    }
  }

  return resultString.str();
}

unsigned int
mitk::DICOMTagBasedSorter::CutDecimalPlaces
::GetPrecision() const
{
  return m_Precision;
}

mitk::DICOMTagBasedSorter
::DICOMTagBasedSorter()
:DICOMDatasetSorter()
{
}

mitk::DICOMTagBasedSorter
::~DICOMTagBasedSorter()
{
  for(TagValueProcessorMap::iterator ti = m_TagValueProcessor.begin();
      ti != m_TagValueProcessor.end();
      ++ti)
  {
    delete ti->second;
  }
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

bool
mitk::DICOMTagBasedSorter
::operator==(const DICOMDatasetSorter& other) const
{
  if (const DICOMTagBasedSorter* otherSelf = dynamic_cast<const DICOMTagBasedSorter*>(&other))
  {
    return true; // TODO
  }
  else
  {
    return false;
  }
}

void
mitk::DICOMTagBasedSorter
::PrintConfiguration(std::ostream& os, const std::string& indent) const
{
  os << indent << "Tag based sorting:" << std::endl;
  for (DICOMTagList::const_iterator tagIter = m_DistinguishingTags.begin();
       tagIter != m_DistinguishingTags.end();
       ++tagIter)
  {
    os << indent << "  Split on ";
    tagIter->Print(os);
    os << std::endl;
  }

  DICOMSortCriterion::ConstPointer crit = m_SortCriterion.GetPointer();
  while (crit.IsNotNull())
  {
    os << indent << "   Sort by ";
    crit->Print(os);
    os << std::endl;
    crit = crit->GetSecondaryCriterion();
  }
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

mitk::DICOMTagList
mitk::DICOMTagBasedSorter
::GetDistinguishingTags() const
{
  return m_DistinguishingTags;
}

const mitk::DICOMTagBasedSorter::TagValueProcessor*
mitk::DICOMTagBasedSorter
::GetTagValueProcessorForDistinguishingTag(const DICOMTag& tag) const
{
  TagValueProcessorMap::const_iterator loc = m_TagValueProcessor.find(tag);
  if (loc != m_TagValueProcessor.end())
  {
    return loc->second;
  }
  else
  {
    return NULL;
  }
}

void
mitk::DICOMTagBasedSorter
::AddDistinguishingTag( const DICOMTag& tag, TagValueProcessor* tagValueProcessor )
{
  m_DistinguishingTags.push_back(tag);
  m_TagValueProcessor[tag] = tagValueProcessor;
}

void
mitk::DICOMTagBasedSorter
::SetSortCriterion( DICOMSortCriterion::ConstPointer criterion )
{
  m_SortCriterion = criterion;
}

mitk::DICOMSortCriterion::ConstPointer
mitk::DICOMTagBasedSorter
::GetSortCriterion() const
{
  return m_SortCriterion;
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
    std::string rawTagValue = dataset->GetTagValueAsString(*tagIter);
    std::string processedTagValue;
    if ( m_TagValueProcessor[*tagIter] != NULL )
    {
      processedTagValue = (*m_TagValueProcessor[*tagIter])(rawTagValue);
    }
    else
    {
      processedTagValue = rawTagValue;
    }
    groupID << processedTagValue;
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

  MITK_DEBUG << "After tag based splitting: " << listForGroupID.size() << " groups";

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
    unsigned int groupIndex(0);
    for (GroupIDToListType::iterator gIter = groups.begin();
         gIter != groups.end();
         ++groupIndex, ++gIter)
    {
      DICOMDatasetList& dsList = gIter->second;
      MITK_DEBUG << "   --------------------------------------------------------------------------------";
      MITK_DEBUG << "   DICOMTagBasedSorter before sorting group : " << groupIndex;
      for (DICOMDatasetList::iterator oi = dsList.begin();
           oi != dsList.end();
           ++oi)
      {
        MITK_DEBUG << "     INPUT     : " << (*oi)->GetFilenameIfAvailable();
      }

      std::sort( dsList.begin(), dsList.end(), ParameterizedDatasetSort( m_SortCriterion ) );

      MITK_DEBUG << "   --------------------------------------------------------------------------------";
      MITK_DEBUG << "   DICOMTagBasedSorter after sorting group : " << groupIndex;
      for (DICOMDatasetList::iterator oi = dsList.begin();
           oi != dsList.end();
           ++oi)
      {
        MITK_DEBUG << "     OUTPUT    : " << (*oi)->GetFilenameIfAvailable();
      }
      MITK_DEBUG << "   --------------------------------------------------------------------------------";
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
