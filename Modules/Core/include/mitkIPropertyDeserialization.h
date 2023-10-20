/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIPropertyDeserialization_h
#define mitkIPropertyDeserialization_h

#include <mitkBaseProperty.h>
#include <mitkServiceInterface.h>
#include <MitkCoreExports.h>

#include <itkSmartPointer.h>

#include <string>
#include <type_traits>

namespace mitk
{
  class MITKCORE_EXPORT IPropertyDeserialization
  {
  public:
    virtual ~IPropertyDeserialization();

    virtual BaseProperty::Pointer CreateInstance(const std::string& className) = 0;

    template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseProperty, T>>>
    void RegisterProperty()
    {
      this->InternalRegisterProperty(T::New());
    }

  protected:

    virtual void InternalRegisterProperty(const BaseProperty* property) = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyDeserialization, "org.mitk.IPropertyDeserialization")

#endif
