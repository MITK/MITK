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

#include "mitkVectorProperty.h"

namespace mitk
{

template <typename DATATYPE>
bool VectorProperty<DATATYPE>::IsEqual(const BaseProperty& property) const
{
  return this->m_PropertyContent == static_cast<const Self&>(property).m_PropertyContent;
}


template <typename DATATYPE>
bool VectorProperty<DATATYPE>::Assign(const BaseProperty& property)
{
  this->m_PropertyContent = static_cast<const Self&>(property).m_PropertyContent;
  return true;
}


template <typename DATATYPE>
itk::LightObject::Pointer VectorProperty<DATATYPE>::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  return result;
}


template <typename DATATYPE>
std::string VectorProperty<DATATYPE>::GetValueAsString() const
{
  std::stringstream value_collector;
  for ( int i = 0; i < m_PropertyContent.size(); i++ )
  {
      value_collector << m_PropertyContent[i] << ",";
  }
  return value_collector.str();
}


template <typename DATATYPE>
void VectorProperty<DATATYPE>::SetValue(const VectorType& newValue)
{
  m_PropertyContent = newValue;
}


template <typename DATATYPE>
const typename VectorProperty<DATATYPE>::VectorType& VectorProperty<DATATYPE>::GetValue() const
{
  return m_PropertyContent;
}

// Explicit instantiation for defined types.
MITK_DEFINE_VECTOR_PROPERTY(double)
MITK_DEFINE_VECTOR_PROPERTY(int)

} // namespace mitk
