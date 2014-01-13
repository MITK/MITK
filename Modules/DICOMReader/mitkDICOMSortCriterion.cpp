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

#include "mitkDICOMSortCriterion.h"

mitk::DICOMSortCriterion
::DICOMSortCriterion(DICOMSortCriterion::Pointer secondaryCriterion)
:itk::LightObject()
,m_SecondaryCriterion(secondaryCriterion)
{
}

mitk::DICOMSortCriterion
::~DICOMSortCriterion()
{
}

mitk::DICOMSortCriterion
::DICOMSortCriterion(const DICOMSortCriterion& other )
:itk::LightObject()
,m_SecondaryCriterion(other.m_SecondaryCriterion)
{
}

mitk::DICOMSortCriterion&
mitk::DICOMSortCriterion
::operator=(const DICOMSortCriterion& other)
{
  if (this != &other)
  {
    m_SecondaryCriterion = other.m_SecondaryCriterion;
  }
  return *this;
}

mitk::DICOMSortCriterion::ConstPointer
mitk::DICOMSortCriterion
::GetSecondaryCriterion() const
{
  return m_SecondaryCriterion.GetPointer();
}

mitk::DICOMTagList
mitk::DICOMSortCriterion
::GetAllTagsOfInterest() const
{
  DICOMTagList allTags;

 // iterate all secondary criteria, return all tags
  const DICOMSortCriterion* criterionToCheck = this;
  while (criterionToCheck)
  {
    DICOMTagList newElements = criterionToCheck->GetTagsOfInterest();
    allTags.insert( allTags.end(), newElements.begin(), newElements.end() ); // append
    criterionToCheck = criterionToCheck->m_SecondaryCriterion.GetPointer();
  }

  return allTags;
}

bool
mitk::DICOMSortCriterion
::NextLevelIsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const
{
  if (m_SecondaryCriterion.IsNotNull())
  {
    return m_SecondaryCriterion->IsLeftBeforeRight(left, right);
  }
  else
  {
    return (void*)left < (void*)right;
  }
}
