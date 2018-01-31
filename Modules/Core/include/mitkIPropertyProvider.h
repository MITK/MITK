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

#ifndef mitkIPropertyProvider_h
#define mitkIPropertyProvider_h

#include <MitkCoreExports.h>
#include <mitkBaseProperty.h>

namespace mitk
{
  /** Interface implemented by classes that provide access to property information.
   @remark: This classes may not be the owner of the property, thus only grants const access to it.
   See IPropertyOwner for ownership.*/
  class MITKCORE_EXPORT IPropertyProvider
  {
  public:
    virtual ~IPropertyProvider();

    /**
     * \brief Get property by its key.
     *
     * \param[in] propertyKey Key of property.
     * \param[in] contextName Optional, default is empty string (default
     *            context). Search in specified context.
     * \param[in] fallBackOnDefaultContext Optional, default is true. Also
     *            search in default context if property was not found in given
     *            context.
     *
     * \return Found property, nullptr otherwise.
     */
    virtual BaseProperty::ConstPointer GetConstProperty(const std::string &propertyKey,
                                                        const std::string &contextName = "",
                                                        bool fallBackOnDefaultContext = true) const = 0;

    /**
     * \brief Query keys of existing properties.
     *
     * \param[in] contextName Optional, default is empty string (default
     *            context). Search in specified context.
     * \param[in] includeDefaultContext Optional, default is false. Include
     *            default context.
     *
     * \return List of property keys.
     */
    virtual std::vector<std::string> GetPropertyKeys(const std::string &contextName = "",
                                                     bool includeDefaultContext = false) const = 0;

    /**
     * \brief Query names of existing contexts.
     *
     * \return List of context names.
     */
    virtual std::vector<std::string> GetPropertyContextNames() const = 0;
  };
} // namespace mitk

#endif
