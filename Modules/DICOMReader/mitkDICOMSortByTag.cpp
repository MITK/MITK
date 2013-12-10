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

#include "mitkDICOMSortByTag.h"

mitk::DICOMSortByTag
::DICOMSortByTag(const DICOMTag& tag, DICOMSortCriterion::Pointer secondaryCriterion)
:DICOMSortCriterion(secondaryCriterion)
,m_Tag(tag)
{
}

mitk::DICOMSortByTag
::~DICOMSortByTag()
{
}

mitk::DICOMSortByTag
::DICOMSortByTag(const DICOMSortByTag& other )
:DICOMSortCriterion(other)
,m_Tag(other.m_Tag)
{
}

mitk::DICOMSortByTag&
mitk::DICOMSortByTag
::operator=(const DICOMSortByTag& other)
{
  if (this != &other)
  {
    DICOMSortCriterion::operator=(other);
    m_Tag = other.m_Tag;
  }
  return *this;
}

mitk::DICOMTagList
mitk::DICOMSortByTag
::GetTagsOfInterest() const
{
  DICOMTagList list;
  list.push_back(m_Tag);
  return list;
}

bool
mitk::DICOMSortByTag
::IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const
{
  return this->NumericCompare(left, right, m_Tag);
}

bool
mitk::DICOMSortByTag
::StringCompare(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right, const DICOMTag& tag) const
{
  assert(left);
  assert(right);

  std::string leftString = left->GetTagValueAsString(tag);
  std::string rightString = right->GetTagValueAsString(tag);

  if (leftString != rightString)
  {
    return leftString.compare(rightString) < 0;
  }
  else
  {
    if (m_SecondaryCriterion.IsNotNull())
    {
      return m_SecondaryCriterion->IsLeftBeforeRight(left, right);
    }
    else
    {
      return leftString.compare(rightString) < 0; // TODO last resort needed
    }
  }
}

bool
mitk::DICOMSortByTag
::NumericCompare(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right, const DICOMTag& tag) const
{
  assert(left);
  assert(right);

  std::string leftString = left->GetTagValueAsString(tag);
  std::string rightString = right->GetTagValueAsString(tag);

  const char* leftInput(leftString.c_str());
  const char* rightInput(rightString.c_str());
  char* leftEnd(NULL);
  char* rightEnd(NULL);

  double leftDouble = strtod(leftInput, &leftEnd);
  double rightDouble = strtod(rightInput, &rightEnd);

  if (leftEnd == leftInput || rightEnd == rightInput) // no numerical conversion..
  {
    return this->StringCompare(left,right, tag); // fallback to string compare
  }
  else
  {
    if (leftDouble != rightDouble) // can we decide?
    {
      return leftDouble < rightDouble;
    }
    else // ask secondary criterion
    {
      if (m_SecondaryCriterion.IsNotNull())
      {
        return m_SecondaryCriterion->IsLeftBeforeRight(left, right);
      }
      else
      {
        return leftDouble < rightDouble; // TODO last resort
      }
    }
  }
}
