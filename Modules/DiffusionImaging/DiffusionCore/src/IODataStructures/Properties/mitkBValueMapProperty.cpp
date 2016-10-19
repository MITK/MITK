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

#include "mitkBValueMapProperty.h"

mitk::BValueMapProperty::BValueMapProperty()
  : m_BValueMap()
{
}

mitk::BValueMapProperty::BValueMapProperty(const BValueMapProperty& other)
  : mitk::BaseProperty(other)
{
  m_BValueMap = other.GetBValueMap();
}

mitk::BValueMapProperty::BValueMapProperty(const BValueMap& bValueMap)
{
  m_BValueMap = bValueMap;
}

mitk::BValueMapProperty::~BValueMapProperty()
{
}

const mitk::BValueMapProperty::BValueMap & mitk::BValueMapProperty::GetBValueMap() const
{
  return m_BValueMap;
}

void mitk::BValueMapProperty::SetBValueMap(const BValueMap & map)
{
  this->m_BValueMap = map;
}

bool mitk::BValueMapProperty::IsEqual(const BaseProperty& property) const
{
  return this->m_BValueMap == static_cast<const Self&>(property).m_BValueMap;
}

bool mitk::BValueMapProperty::Assign(const BaseProperty& property)
{
  this->m_BValueMap = static_cast<const Self&>(property).m_BValueMap;
  return true;
}

mitk::BValueMapProperty::BValueMap mitk::BValueMapProperty::CreateBValueMap(const mitk::BValueMapProperty::GradientDirectionsContainerType * gdc, float referenceBValue)
{
  mitk::BValueMapProperty::BValueMap map;

  mitk::BValueMapProperty::GradientDirectionsContainerType::ConstIterator gdcit;
  for( gdcit = gdc->Begin(); gdcit != gdc->End(); ++gdcit)
  {
    float keyBValue = mitk::BValueMapProperty::GetBValueOfGradientDirection(gdcit.Index(),referenceBValue, gdc);
    map[keyBValue].push_back(gdcit.Index());
  }

  return map;
}


float mitk::BValueMapProperty::GetBValueOfGradientDirection(unsigned int i, float referenceBValue, const mitk::BValueMapProperty::GradientDirectionsContainerType * gdc)
{
  if(i > gdc->Size()-1)
    return -1;

  if(gdc->ElementAt(i).one_norm() <= 0.0)
  {
    return 0;
  }
  else
  {
    double twonorm = gdc->ElementAt(i).two_norm();
    double bval = referenceBValue*twonorm*twonorm;

    if (bval<0)
      bval = ceil(bval - 0.5);
    else
      bval = floor(bval + 0.5);

    return bval;
  }
}

itk::LightObject::Pointer  mitk::BValueMapProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
