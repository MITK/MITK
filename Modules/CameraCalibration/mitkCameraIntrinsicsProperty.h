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

#ifndef MITKCAMERAINTRINSICSPROPERTY_H
#define MITKCAMERAINTRINSICSPROPERTY_H

#include "mitkBaseProperty.h"
#include "mitkCameraIntrinsics.h"


namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

class mitkCameraCalibration_EXPORT CameraIntrinsicsProperty : public BaseProperty
{
public:

  typedef mitk::CameraIntrinsics::Pointer ValueType;

  mitkClassMacro(CameraIntrinsicsProperty, BaseProperty);

  itkNewMacro(CameraIntrinsicsProperty);
  mitkNewMacro1Param(CameraIntrinsicsProperty, mitk::CameraIntrinsics::Pointer);

  itkSetMacro(Value, mitk::CameraIntrinsics::Pointer );
  itkGetConstMacro(Value, mitk::CameraIntrinsics::Pointer );

  std::string GetValueAsString() const;

  using BaseProperty::operator=;

protected:
  mitk::CameraIntrinsics::Pointer m_Value;

  CameraIntrinsicsProperty();

  CameraIntrinsicsProperty( mitk::CameraIntrinsics::Pointer value );

private:

  // purposely not implemented
  CameraIntrinsicsProperty(const CameraIntrinsicsProperty&);
  CameraIntrinsicsProperty& operator=(const CameraIntrinsicsProperty&);

  virtual bool IsEqual(const BaseProperty& property) const;
  virtual bool Assign(const BaseProperty& property);
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk

#endif // MITKCAMERAINTRINSICSPROPERTY_H
