/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCAMERAINTRINSICSPROPERTY_H
#define MITKCAMERAINTRINSICSPROPERTY_H

#include "mitkBaseProperty.h"
#include "mitkCameraIntrinsics.h"


namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

class MITKCAMERACALIBRATION_EXPORT CameraIntrinsicsProperty : public BaseProperty
{
public:

  typedef mitk::CameraIntrinsics::Pointer ValueType;

  mitkClassMacro(CameraIntrinsicsProperty, BaseProperty);

  itkFactorylessNewMacro(Self);

  itkCloneMacro(Self)
  mitkNewMacro1Param(CameraIntrinsicsProperty, mitk::CameraIntrinsics::Pointer);

  itkSetMacro(Value, mitk::CameraIntrinsics::Pointer );
  itkGetConstMacro(Value, mitk::CameraIntrinsics::Pointer );

  std::string GetValueAsString() const override;

  using BaseProperty::operator=;

protected:
  mitk::CameraIntrinsics::Pointer m_Value;

  CameraIntrinsicsProperty();

  CameraIntrinsicsProperty(const CameraIntrinsicsProperty&);

  CameraIntrinsicsProperty( mitk::CameraIntrinsics::Pointer value );

private:

  // purposely not implemented
  CameraIntrinsicsProperty& operator=(const CameraIntrinsicsProperty&);

  itk::LightObject::Pointer InternalClone() const override;

  bool IsEqual(const BaseProperty& property) const override;
  bool Assign(const BaseProperty& property) override;
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk

#endif // MITKCAMERAINTRINSICSPROPERTY_H
