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

#ifndef mitkIPropertyFilters_h
#define mitkIPropertyFilters_h

#include <mitkBaseProperty.h>
#include <mitkPropertyFilter.h>
#include <usServiceInterface.h>
#include <MitkExports.h>

namespace mitk
{
  /** \brief Interface of property filters service.
    *
    * This service allows you set property name filters.
    * A mitk::PropertyFilter consist of blacklist and whitelist entries and can be restricted to certain data node types.
    * The property view only displays filtered properties if filters were found.
    */
  class MITK_CORE_EXPORT IPropertyFilters
  {
  public:
    virtual ~IPropertyFilters();

    virtual bool AddFilter(const PropertyFilter& filter, const std::string& className = "", bool overwrite = false) = 0;
    virtual std::map<std::string, BaseProperty::Pointer> ApplyFilter(const std::map<std::string, BaseProperty::Pointer>& propertyMap, const std::string& className = "") const = 0;
    virtual PropertyFilter GetFilter(const std::string& className = "") const = 0;
    virtual bool HasFilter(const std::string& className = "") const = 0;
    virtual void RemoveAllFilters() = 0;
    virtual void RemoveFilter(const std::string& className = "") = 0;
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::IPropertyFilters, "org.mitk.IPropertyFilters")

#endif
