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
#include <mitkServiceInterface.h>
#include <map>
#include <string>
#include <MitkCoreExports.h>

namespace mitk
{
  class PropertyFilter;

  /** \brief Interface of property filters service.
    *
    * This service allows you manage property name filters.
    * A mitk::PropertyFilter consist of blacklist and whitelist entries and can be restricted to certain data node types.
    * The property view only displays filtered properties if filters were found.
    */
  class MITK_CORE_EXPORT IPropertyFilters
  {
  public:
    virtual ~IPropertyFilters();

    /** \brief Add a property filter.
      *
      * \param[in] filter The property filter.
      * \param[in] className Optional data node class name to which the filter is restricted.
      * \param[in] overwrite Overwrite already existing filter.
      * \return True if filter was added auccessfully.
      */
    virtual bool AddFilter(const PropertyFilter& filter, const std::string& className = "", bool overwrite = false) = 0;

    /** \brief Apply property filter to property list.
      *
      * \param[in] propertyMap Property list to which the filter is applied.
      * \param[in] className Optional data node class name for which the filter is chosen.
      * \return Filtered property list.
      */
    virtual std::map<std::string, BaseProperty::Pointer> ApplyFilter(const std::map<std::string, BaseProperty::Pointer>& propertyMap, const std::string& className = "") const = 0;

    /** \brief Get the current property filter.
      *
      * \param[in] className Optional data node class name for which the filter is returned.
      * \return The current property filter.
      */
    virtual PropertyFilter GetFilter(const std::string& className = "") const = 0;

    /** \brief Check if a specific data node class name has a property filter.
      *
      * \param[in] className The data node class name or empty string for global property filter.
      * \return True if data node class name has a filter, false otherwise.
      */
    virtual bool HasFilter(const std::string& className = "") const = 0;

    /** \brief Remove all property filters.
      */
    virtual void RemoveAllFilters() = 0;

    /** \brief Remove property filter for a specific data node class name.
      *
      * \param[in] className The data node class name or empty string for global property filter.
      */
    virtual void RemoveFilter(const std::string& className = "") = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyFilters, "org.mitk.IPropertyFilters")

#endif
