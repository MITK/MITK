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

#include <mitkServiceInterface.h>
#include <string>
#include <vector>
#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Interface of property aliases service.
    *
    * This service allows you to manage aliases of property names.
    * A property name can be mapped to more than one alias and aliases can be restricted to specific data node types.
    * The property view prefers to display aliases instead of genuine property names.
    */
  class MITK_CORE_EXPORT IPropertyAliases
  {
  public:
    virtual ~IPropertyAliases();

    /** \brief Add an alias for a specific property.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] alias Alias for the property.
      * \param[in] className Optional data node class name to which this alias is restricted.
      * \return False if alias is an empty string.
      */
    virtual bool AddAlias(const std::string& propertyName, const std::string& alias, const std::string& className = "") = 0;

    /** \brief Get aliases for a specific property.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] className Optional data node class name to which the returned aliases are restricted.
      * \return Aliases that match the input criteria.
      */
    virtual std::vector<std::string> GetAliases(const std::string& propertyName, const std::string& className = "") = 0;

    /** \brief Get property name that is associated to specific alias.
      *
      * \param[in] alias Alias of the property.
      * \param[in] className Optional data node class name to which the alias is restricted.
      * \return Associated property name or empty string if no property name was found.
      */
    virtual std::string GetPropertyName(const std::string& alias, const std::string& className = "") = 0;

    /** \brief Check if a specific property has aliases.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] className Optional data node class name to which the aliases are restricted.
      * \return True if the property has aliases, false otherwise.
      */
    virtual bool HasAliases(const std::string& propertyName, const std::string& className = "") = 0;

    /** \brief Remove specific alias of a certain property.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] alias Alias of the property.
      * \param[in] className Optional data node class name to which the alias is restricted.
      */
    virtual void RemoveAlias(const std::string& propertyName, const std::string& alias, const std::string& className = "") = 0;

    /** \brief Remove all aliases of a specific property.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] className Optional data node class name to which the aliases are restricted.
      */
    virtual void RemoveAliases(const std::string& propertyName, const std::string& className = "") = 0;

    /** \brief Remove all aliases of all properties.
      *
      * \param[in] className Optional data node class name to which the removal is restricted.
      */
    virtual void RemoveAllAliases(const std::string& className = "") = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyAliases, "org.mitk.IPropertyAliases")

#endif
