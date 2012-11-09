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


#include "mitkTransferFunctionProperty.h"


namespace mitk {

bool TransferFunctionProperty::IsEqual(const BaseProperty& property) const
{
  return *(this->m_Value) == *(static_cast<const Self&>(property).m_Value);
}

bool TransferFunctionProperty::Assign(const BaseProperty& property)
{
  this->m_Value = static_cast<const Self&>(property).m_Value;
  return true;
}

std::string  TransferFunctionProperty::GetValueAsString() const
{
  std::stringstream myStr;
  myStr << GetValue();
  return myStr.str();
}

TransferFunctionProperty::TransferFunctionProperty()
  : BaseProperty()
{}

TransferFunctionProperty::TransferFunctionProperty( mitk::TransferFunction::Pointer value )
  : BaseProperty(), m_Value( value )
{}

} // namespace mitk
