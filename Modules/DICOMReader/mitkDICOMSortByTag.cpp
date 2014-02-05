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

bool
mitk::DICOMSortByTag
::operator==(const DICOMSortCriterion& other) const
{
  if (const DICOMSortByTag* otherSelf = dynamic_cast<const DICOMSortByTag*>(&other))
  {
    if (!(this->m_Tag == otherSelf->m_Tag)) return false;

    if (this->m_SecondaryCriterion.IsNull() && otherSelf->m_SecondaryCriterion.IsNull()) return true;

    if (this->m_SecondaryCriterion.IsNull() || otherSelf->m_SecondaryCriterion.IsNull()) return false;

    return *(this->m_SecondaryCriterion) == *(otherSelf->m_SecondaryCriterion);
  }
  else
  {
    return false;
  }
}

void
mitk::DICOMSortByTag
::Print(std::ostream& os) const
{
  m_Tag.Print(os);
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
    return this->NextLevelIsLeftBeforeRight(left, right);
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

  std::istringstream lefti(leftString);
  std::istringstream righti(rightString);

  double leftDouble(0);
  double rightDouble(0);

  if (    (lefti >> leftDouble) && (righti >> rightDouble)
       && lefti.eof() && righti.eof() )
  {
    if (leftDouble != rightDouble) // can we decide?
    {
      return leftDouble < rightDouble;
    }
    else // ask secondary criterion
    {
      return this->NextLevelIsLeftBeforeRight(left, right);
    }
  }
  else // no numerical conversion..
  {
    return this->StringCompare(left,right, tag); // fallback to string compare
  }
}

double
mitk::DICOMSortByTag
::NumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to) const
{
  assert(from);
  assert(to);

  std::string fromString = from->GetTagValueAsString(m_Tag);
  std::string toString = to->GetTagValueAsString(m_Tag);

  std::istringstream fromi(fromString);
  std::istringstream toi(toString);

  double fromDouble(0);
  double toDouble(0);

  if (    (fromi >> fromDouble) && (toi >> toDouble)
       && fromi.eof() && toi.eof() )
  {
    return toDouble - fromDouble;
  }
  else
  {
    MITK_WARN << "NO NUMERIC DISTANCE between '" << fromString << "' and '" << toString << "'";
    return 0;
  }

  // TODO second-level compare?
}
