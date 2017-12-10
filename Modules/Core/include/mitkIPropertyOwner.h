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

#ifndef mitkIPropertyOwner_h
#define mitkIPropertyOwner_h

#include <mitkBaseProperty.h>
#include <MitkCoreExports.h>

namespace mitk
{
  class MITKCORE_EXPORT IPropertyOwner
  {
  public:
    using ContextType = std::string;
    using KeyType = std::string;

    virtual ~IPropertyOwner();

    /**
     * \brief Get property by its key.
     *
     * \param[in] key Property key/name.
     * \param[in] context Optional, default is empty string. Search in specific context first.
     * \param[in] strict Optional, default is false. Restrict search to specified context and ignore default context.
     *
     * \return Found property, nullptr otherwise.
     */
    virtual BaseProperty * GetProperty(const KeyType &key, const ContextType &context = "", bool strict = false) const = 0;

    /**
     * \brief Add new or change existent property.
     *
     * \param[in] key Property key/name.
     * \param[in] property Property/value.
     * \param[in] context Optional, default is empty string. Context in which the property is set.
     *
     */
    virtual void SetProperty(const KeyType &key, BaseProperty *property, const ContextType &context = "") = 0;
  };
}

#endif
