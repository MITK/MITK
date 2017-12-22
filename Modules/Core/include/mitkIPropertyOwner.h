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

#include <mitkIPropertyProvider.h>
#include <MitkCoreExports.h>

namespace mitk
{
  class MITKCORE_EXPORT IPropertyOwner : public IPropertyProvider
  {
  public:
    ~IPropertyOwner() override;

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
    virtual BaseProperty * GetNonConstProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = true) = 0;

    /**
    * \brief Add new or change existent property.
    *
    * \param[in] propertyKey Key of property.
    * \param[in] property The actual property.
    * \param[in] contextName Optional, default is empty string (default
                 context). Context in which the property is set.
    * \param[in] fallBackOnDefaultContext Optional, default is false. Set
    *            property in default context if given context does not
    *            exist.
    */
    virtual void SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName = "", bool fallBackOnDefaultContext = false) = 0;
  };
}

#endif
