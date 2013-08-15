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

#ifndef mitkIPropertyAliases_h
#define mitkIPropertyAliases_h

#include <itkLightObject.h>
#include <usServiceInterface.h>
#include <vector>
#include <MitkExports.h>

namespace mitk
{
  /** \brief Interface of property aliases service.
    *
    * This service allows you to set aliases for property names.
    * A property name can be mapped to more than one alias and aliases can be restricted to certain data node types.
    * The property view prefers to display aliases instead of genuine property names.
    */
  class MITK_CORE_EXPORT IPropertyAliases
  {
  public:
    virtual ~IPropertyAliases();

    virtual bool AddAlias(const std::string& propertyName, const std::string& alias, const std::string& className = "") = 0;
    virtual std::vector<std::string> GetAliases(const std::string& propertyName, const std::string& className = "") = 0;
    virtual std::string GetPropertyName(const std::string& alias, const std::string& className = "") = 0;
    virtual bool HasAliases(const std::string& propertyName, const std::string& className = "") = 0;
    virtual void RemoveAlias(const std::string& propertyName, const std::string& alias, const std::string& className = "") = 0;
    virtual void RemoveAliases(const std::string& propertyName, const std::string& className = "") = 0;
    virtual void RemoveAllAliases(const std::string& className = "") = 0;
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::IPropertyAliases, "org.mitk.IPropertyAliases")

#endif
