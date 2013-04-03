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

#ifndef mitkPropertyFilters_h
#define mitkPropertyFilters_h

#include "mitkPropertyFilter.h"
#include <mitkBaseProperty.h>
#include <usServiceInterface.h>

namespace mitk
{
  class PropertyFilters
  {
  public:
    virtual ~PropertyFilters();

    virtual bool AddFilter(const PropertyFilter& filter, bool overwrite = false) = 0;
    virtual bool AddFilter(const std::string& className, const PropertyFilter& filter, bool overwrite = false) = 0;
    virtual std::map<std::string, BaseProperty::Pointer> ApplyFilter(const std::map<std::string, BaseProperty::Pointer>& propertyMap) const = 0;
    virtual std::map<std::string, BaseProperty::Pointer> ApplyFilter(const std::string& className, const std::map<std::string, BaseProperty::Pointer>& propertyMap) const = 0;
    virtual PropertyFilter GetFilter(const std::string& className = "") const = 0;
    virtual bool HasFilter(const std::string& className = "") const = 0;
    virtual void RemoveAllFilters() = 0;
    virtual void RemoveFilter(const std::string& className = "") = 0;
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::PropertyFilters, "org.mitk.services.PropertyFilters")

#endif
