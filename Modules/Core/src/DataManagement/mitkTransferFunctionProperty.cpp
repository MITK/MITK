/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTransferFunctionProperty.h"

namespace mitk
{
  bool TransferFunctionProperty::IsEqual(const BaseProperty &property) const
  {
    return *(this->m_Value) == *(static_cast<const Self &>(property).m_Value);
  }

  bool TransferFunctionProperty::Assign(const BaseProperty &property)
  {
    this->m_Value = static_cast<const Self &>(property).m_Value;
    return true;
  }

  std::string TransferFunctionProperty::GetValueAsString() const
  {
    std::stringstream myStr;
    myStr << GetValue();
    return myStr.str();
  }

  TransferFunctionProperty::TransferFunctionProperty() : BaseProperty(), m_Value(mitk::TransferFunction::New()) {}
  TransferFunctionProperty::TransferFunctionProperty(const TransferFunctionProperty &other)
    : BaseProperty(other), m_Value(other.m_Value->Clone())
  {
  }

  TransferFunctionProperty::TransferFunctionProperty(mitk::TransferFunction::Pointer value)
    : BaseProperty(), m_Value(value)
  {
  }

  itk::LightObject::Pointer TransferFunctionProperty::InternalClone() const
  {
    itk::LightObject::Pointer result(new Self(*this));
    result->UnRegister();
    return result;
  }

} // namespace mitk
