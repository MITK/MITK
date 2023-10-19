/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIPropertyDeserialization_h
#define mitkIPropertyDeserialization_h

#include <MitkCoreExports.h>
#include <mitkServiceInterface.h>
#include <itkSmartPointer.h>
#include <string>

namespace mitk
{
  class BaseProperty;

  class MITKCORE_EXPORT IPropertyDeserialization
  {
  public:
    virtual ~IPropertyDeserialization();

    virtual void RegisterProperty(const BaseProperty* property) = 0;
    virtual itk::SmartPointer<BaseProperty> CreateInstance(const std::string& className) = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyDeserialization, "org.mitk.IPropertyDeserialization")

#endif
