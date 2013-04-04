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

#ifndef mitkPropertyDescriptions_h
#define mitkPropertyDescriptions_h

#include <itkLightObject.h>
#include <PropertiesExports.h>
#include <usServiceInterface.h>

namespace mitk
{
  class Properties_EXPORT PropertyDescriptions
  {
  public:
    virtual ~PropertyDescriptions();

    virtual bool AddDescription(const std::string& propertyName, const std::string& description, bool overwrite = false) = 0;
    virtual std::string GetDescription(const std::string& propertyName) const = 0;
    virtual bool HasDescription(const std::string& propertyName) const = 0;
    virtual void RemoveAllDescriptions() = 0;
    virtual void RemoveDescription(const std::string& propertyName) = 0;
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::PropertyDescriptions, "org.mitk.services.PropertyDescriptions")

#endif
