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

#include "ofstd.h"

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

  const std::string leftString = left->GetTagValueAsString(tag);
  const std::string rightString = right->GetTagValueAsString(tag);

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

  const std::string leftString = left->GetTagValueAsString(tag);
  const std::string rightString = right->GetTagValueAsString(tag);

  double leftDouble( 0 );
  double rightDouble( 0 );

  try
  {
    leftDouble = OFStandard::atof( leftString.c_str() );
    rightDouble = OFStandard::atof( rightString.c_str() );
  }
  catch ( const std::exception& /*e*/ )
  {
    return this->StringCompare(left,right, tag); // fallback to string compare
  }


  if ( leftDouble != rightDouble ) // can we decide?
  {
    return leftDouble < rightDouble;
  }
  else // ask secondary criterion
  {
    return this->NextLevelIsLeftBeforeRight( left, right );
  }
}

double
mitk::DICOMSortByTag
::NumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to) const
{
  assert(from);
  assert(to);

  const std::string fromString = from->GetTagValueAsString(m_Tag);
  const std::string toString = to->GetTagValueAsString(m_Tag);

  double fromDouble(0);
  double toDouble(0);

  try
  {
    fromDouble = OFStandard::atof( fromString.c_str() );
    toDouble = OFStandard::atof( toString.c_str() );
  }
  catch ( const std::exception& /*e*/ )
  {
    MITK_WARN << "NO NUMERIC DISTANCE between '" << fromString << "' and '" << toString << "'";
    return 0;
  }

  return toDouble - fromDouble;
  // TODO second-level compare?
}
