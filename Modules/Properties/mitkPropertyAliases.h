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

#ifndef mitkPropertyAliases_h
#define mitkPropertyAliases_h

#include <itkLightObject.h>
#include <usServiceInterface.h>

namespace mitk
{
  class PropertyAliases
  {
  public:
    virtual ~PropertyAliases();

    virtual bool AddAlias(const std::string& propertyName, const std::string& alias, bool overwrite = false) = 0;
    virtual std::string GetAlias(const std::string& propertyName) const = 0;
    virtual std::string GetPropertyName(const std::string& alias) const = 0;
    virtual bool HasAlias(const std::string& propertyName) const = 0;
    virtual void RemoveAlias(const std::string& propertyName) = 0;
    virtual void RemoveAllAliases() = 0;
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::PropertyAliases, "org.mitk.services.PropertyAliases")

#endif
