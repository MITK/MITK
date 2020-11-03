/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkCameraIntrinsicsProperty.h"


namespace mitk {

bool CameraIntrinsicsProperty::IsEqual(const BaseProperty& property) const
{
  return this->m_Value->Equals(static_cast<const Self&>(property).m_Value.GetPointer());
}

bool CameraIntrinsicsProperty::Assign(const BaseProperty& property)
{
  this->m_Value = static_cast<const Self&>(property).m_Value;
  return true;
}

std::string  CameraIntrinsicsProperty::GetValueAsString() const
{
  std::stringstream myStr;
  myStr << GetValue();
  return myStr.str();
}

CameraIntrinsicsProperty::CameraIntrinsicsProperty()
  : BaseProperty()
{}

CameraIntrinsicsProperty::CameraIntrinsicsProperty(const CameraIntrinsicsProperty& other)
  : BaseProperty(other)
{

}

CameraIntrinsicsProperty::CameraIntrinsicsProperty( mitk::CameraIntrinsics::Pointer value )
  : BaseProperty(), m_Value( value )
{}

itk::LightObject::Pointer CameraIntrinsicsProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}

}
// namespace mitk
