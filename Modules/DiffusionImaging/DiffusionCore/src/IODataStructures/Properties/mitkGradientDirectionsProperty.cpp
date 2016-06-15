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

#include "mitkGradientDirectionsProperty.h"

mitk::GradientDirectionsProperty::GradientDirectionsProperty()
{
  m_GradientDirectionsContainer = mitk::GradientDirectionsProperty::GradientDirectionsContainerType::New();
}

mitk::GradientDirectionsProperty::GradientDirectionsProperty(const GradientDirectionsProperty& other)
  : mitk::BaseProperty(other)
{
  m_GradientDirectionsContainer = other.GetGradientDirectionsContainer();
}

mitk::GradientDirectionsProperty::GradientDirectionsProperty(const GradientDirectionsContainerType::Pointer gradientDirectionsContainer)
{
  m_GradientDirectionsContainer = gradientDirectionsContainer;
}

mitk::GradientDirectionsProperty::GradientDirectionsProperty(const AlternativeGradientDirectionsContainerType gradientDirectionsContainer)
{
  m_GradientDirectionsContainer = mitk::GradientDirectionsProperty::GradientDirectionsContainerType::New();
  for(unsigned int index(0); index < gradientDirectionsContainer.size(); index++)
  {
    GradientDirectionType newDirection = gradientDirectionsContainer.at(index).GetVnlVector();
    m_GradientDirectionsContainer->InsertElement( index, newDirection);
  }
}

mitk::GradientDirectionsProperty::~GradientDirectionsProperty()
{
}

const mitk::GradientDirectionsProperty::GradientDirectionsContainerType::Pointer mitk::GradientDirectionsProperty::GetGradientDirectionsContainer() const
{
  return m_GradientDirectionsContainer;
}

bool mitk::GradientDirectionsProperty::IsEqual(const BaseProperty& property) const
{

  GradientDirectionsContainerType::Pointer lhs = this->m_GradientDirectionsContainer;
  GradientDirectionsContainerType::Pointer rhs = static_cast<const Self&>(property).m_GradientDirectionsContainer;

  if(lhs->Size() != rhs->Size()) return false;

  GradientDirectionsContainerType::Iterator lhsit = lhs->Begin();
  GradientDirectionsContainerType::Iterator rhsit = rhs->Begin();

  bool equal = true;
  for(unsigned int i = 0 ; i < lhs->Size(); i++, ++lhsit, ++rhsit)
    equal |= lhsit.Value() == rhsit.Value();

  return  equal;
}

bool mitk::GradientDirectionsProperty::Assign(const BaseProperty& property)
{
  this->m_GradientDirectionsContainer = static_cast<const Self&>(property).m_GradientDirectionsContainer;
  return true;
}

itk::LightObject::Pointer  mitk::GradientDirectionsProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
