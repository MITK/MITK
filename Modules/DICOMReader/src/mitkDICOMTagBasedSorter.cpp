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
#include <iomanip>

mitk::DICOMTagBasedSorter::CutDecimalPlaces
::CutDecimalPlaces(unsigned int precision)
:m_Precision(precision)
{
}

mitk::DICOMTagBasedSorter::CutDecimalPlaces
::CutDecimalPlaces(const CutDecimalPlaces& other)
:m_Precision(other.m_Precision)
{
}

std::string
mitk::DICOMTagBasedSorter::CutDecimalPlaces
::operator()(const std::string& input) const
{
  // be a bit tolerant for tags such as image orientation orienatation, let only the first few digits matter (http://bugs.mitk.org/show_bug.cgi?id=12263)
  // iterate all fields, convert each to a number, cut this number as configured, then return a concatenated string with all cut-off numbers
  std::ostringstream resultString;
  resultString.str(std::string());
  resultString.clear();
  resultString.setf(std::ios::fixed, std::ios::floatfield);
  resultString.precision(m_Precision);

  std::stringstream ss(input);
  ss.str(input);
  ss.clear();
  std::string item;
  double number(0);
  std::istringstream converter(item);
  while (std::getline(ss, item, '\\'))
  {
    converter.str(item);
    converter.clear();
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

mitk::DICOMTagBasedSorter::TagValueProcessor*
mitk::DICOMTagBasedSorter::CutDecimalPlaces
::Clone() const
{
  return new CutDecimalPlaces(*this);
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
,m_StrictSorting(false)
,m_ExpectDistanceOne(false)
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
,m_DistinguishingTags( other.m_DistinguishingTags )
,m_SortCriterion( other.m_SortCriterion )
,m_StrictSorting( other.m_StrictSorting )
,m_ExpectDistanceOne( other.m_ExpectDistanceOne )
{
  for(TagValueProcessorMap::const_iterator ti = other.m_TagValueProcessor.begin();
      ti != other.m_TagValueProcessor.end();
      ++ti)
  {
    m_TagValueProcessor[ti->first] = ti->second->Clone();
  }
}

mitk::DICOMTagBasedSorter&
mitk::DICOMTagBasedSorter
::operator=(const DICOMTagBasedSorter& other)
{
  if (this != &other)
  {
    DICOMDatasetSorter::operator=(other);
    m_DistinguishingTags = other.m_DistinguishingTags;
    m_SortCriterion = other.m_SortCriterion;
    m_StrictSorting = other.m_StrictSorting;
    m_ExpectDistanceOne = other.m_ExpectDistanceOne;

    for(TagValueProcessorMap::const_iterator ti = other.m_TagValueProcessor.begin();
        ti != other.m_TagValueProcessor.end();
        ++ti)
    {
      m_TagValueProcessor[ti->first] = ti->second->Clone();
    }
  }
  return *this;
}

bool
mitk::DICOMTagBasedSorter
::operator==(const DICOMDatasetSorter& other) const
{
  if (const DICOMTagBasedSorter* otherSelf = dynamic_cast<const DICOMTagBasedSorter*>(&other))
  {
    if (this->m_StrictSorting != otherSelf->m_StrictSorting) return false;
    if (this->m_ExpectDistanceOne != otherSelf->m_ExpectDistanceOne) return false;

    bool allTagsPresentAndEqual(true);
    if (this->m_DistinguishingTags.size() != otherSelf->m_DistinguishingTags.size())
      return false;

    for (DICOMTagList::const_iterator myTag = this->m_DistinguishingTags.begin();
        myTag != this->m_DistinguishingTags.end();
        ++myTag)
    {
      allTagsPresentAndEqual &= (std::find( otherSelf->m_DistinguishingTags.begin(), otherSelf->m_DistinguishingTags.end(), *myTag )
                                 != otherSelf->m_DistinguishingTags.end()); // other contains this tags
                                             // since size is equal, we don't need to check the inverse
    }

    if (!allTagsPresentAndEqual) return false;

    if (this->m_SortCriterion.IsNotNull() && otherSelf->m_SortCriterion.IsNotNull())
    {
      return *(this->m_SortCriterion) == *(otherSelf->m_SortCriterion);
    }
    else
    {
      return this->m_SortCriterion.IsNull() && otherSelf->m_SortCriterion.IsNull();
    }
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
  os << indent << "Tag based sorting "
     << "(strict=" << (m_StrictSorting?"true":"false")
     << ", expectDistanceOne=" << (m_ExpectDistanceOne?"true":"false") << "):"
     << std::endl;

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

void
mitk::DICOMTagBasedSorter
::SetStrictSorting(bool strict)
{
  m_StrictSorting = strict;
}

bool
mitk::DICOMTagBasedSorter
::GetStrictSorting() const
{
  return m_StrictSorting;
}

void
mitk::DICOMTagBasedSorter
::SetExpectDistanceOne(bool strict)
{
  m_ExpectDistanceOne = strict;
}

bool
mitk::DICOMTagBasedSorter
::GetExpectDistanceOne() const
{
  return m_ExpectDistanceOne;
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
    /*
       Three steps here:
        1. sort within each group
           - this may result in orders such as 1 2 3 4 6 7 8 10 12 13 14
        2. create new groups by enforcing consecutive order within each group
           - resorts above example like 1 2 3 4  ;  6 7 8  ;  10  ;  12 13 14
        3. sort all of the groups (not WITHIN each group) by their first frame
           - if earlier "distinguish" steps created groups like 6 7 8  ;  1 2 3 4  ; 10,
             then this step would sort them like 1 2 3 4  ;  6 7 8  ;  10
    */

    // Step 1: sort within the groups
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

    GroupIDToListType consecutiveGroups;
    if (m_StrictSorting)
    {
      // Step 2: create new groups by enforcing consecutive order within each group
      unsigned int groupIndex(0);
      for (GroupIDToListType::iterator gIter = groups.begin();
           gIter != groups.end();
           ++gIter)
      {
        std::stringstream groupKey;
        groupKey << std::setfill('0') << std::setw(6) << groupIndex++;

        DICOMDatasetList& dsList = gIter->second;
        DICOMDatasetAccess* previousDS(NULL);
        unsigned int dsIndex(0);
        double constantDistance(0.0);
        bool constantDistanceInitialized(false);
        for (DICOMDatasetList::iterator dataset = dsList.begin();
             dataset != dsList.end();
             ++dsIndex, ++dataset)
        {
          if (dsIndex >0) // ignore the first dataset, we cannot check any distances yet..
          {
            // for the second and every following dataset:
            // let the sorting criterion calculate a "distance"
            // if the distance is not 1, split off a new group!
            double currentDistance = m_SortCriterion->NumericDistance(previousDS, *dataset);
            if (constantDistanceInitialized)
            {
              if (fabs(currentDistance - constantDistance) < fabs(constantDistance * 0.01)) // ok, deviation of up to 1% of distance is tolerated
              {
                // nothing to do, just ok
                MITK_DEBUG << "Checking currentDistance==" << currentDistance << ": small enough";
              }
              //else if (currentDistance < mitk::eps) // close enough to 0
              else
              {
                MITK_DEBUG << "Split consecutive group at index " << dsIndex << " (current distance " << currentDistance << ", constant distance " << constantDistance << ")";
                // split! this is done by simply creating a new group (key)
                groupKey.str(std::string());
                groupKey.clear();
                groupKey << std::setfill('0') << std::setw(6) << groupIndex++;
              }
            }
            else
            {
              // second slice: learn about the expected distance!

              // heuristic: if distance is an integer, we check for a special case:
              //            if the distance is integer and not 1/-1, then we assume
              //            a missing slice right after the first slice
              //            ==> split off slices
              // in all other cases: second dataset at this position, no need to split already, we are still learning about the images

              // addition to the above: when sorting by imagepositions, a distance other than 1 between the first two slices is
              // not unusual, actually expected... then we should not split

              if (m_ExpectDistanceOne)
              {
                if ((currentDistance - (int)currentDistance == 0.0) && fabs(currentDistance) != 1.0)
                  // exact comparison. An integer should not be expressed as 1.000000000000000000000000001!
                {
                  MITK_DEBUG << "Split consecutive group at index " << dsIndex << " (special case: expected distance 1 exactly)";
                  groupKey.str(std::string());
                  groupKey.clear();
                  groupKey << std::setfill('0') << std::setw(6) << groupIndex++;
                }
              }

              MITK_DEBUG << "Initialize strict distance to currentDistance=" << currentDistance;

              constantDistance = currentDistance;
              constantDistanceInitialized = true;
            }
          }
          consecutiveGroups[groupKey.str()].push_back(*dataset);
          previousDS = *dataset;
        }
      }
    }
    else
    {
      consecutiveGroups = groups;
    }

    // Step 3: sort all of the groups (not WITHIN each group) by their first frame
    /*
      build a list-1 of datasets with the first dataset one of each group
      sort this list-1
      build a new result list-2:
       - iterate list-1, for each dataset
         - find the group that contains this dataset
         - add this group as the next element to list-2
      return list-2 as the sorted output
    */
    DICOMDatasetList firstSlices;
    for (GroupIDToListType::iterator gIter = consecutiveGroups.begin();
         gIter != consecutiveGroups.end();
         ++gIter)
    {
      assert(!gIter->second.empty());
      firstSlices.push_back(gIter->second.front());
    }

    std::sort( firstSlices.begin(), firstSlices.end(), ParameterizedDatasetSort( m_SortCriterion ) );

    GroupIDToListType sortedResultBlocks;
    unsigned int groupKeyValue(0);
    for (DICOMDatasetList::iterator firstSlice = firstSlices.begin();
         firstSlice != firstSlices.end();
         ++firstSlice)
    {
      for (GroupIDToListType::iterator gIter = consecutiveGroups.begin();
           gIter != consecutiveGroups.end();
           ++groupKeyValue, ++gIter)
      {
        if (gIter->second.front() == *firstSlice)
        {
          std::stringstream groupKey;
          groupKey << std::setfill('0') << std::setw(6) << groupKeyValue; // try more than 999,999 groups and you are doomed (your application already is)
          sortedResultBlocks[groupKey.str()] = gIter->second;
        }
      }
    }

    groups = sortedResultBlocks;
  }

  unsigned int groupIndex(0);
  for (GroupIDToListType::iterator gIter = groups.begin();
      gIter != groups.end();
      ++groupIndex, ++gIter)
  {
    DICOMDatasetList& dsList = gIter->second;
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
