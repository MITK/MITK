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

#ifndef mitkIPropertyDescriptions_h
#define mitkIPropertyDescriptions_h

#include <itkLightObject.h>
#include <usServiceInterface.h>
#include <MitkExports.h>

namespace mitk
{
  /** \brief Interface of property descriptions service.
    *
    * This service allows you to set a description for a certain property name.
    * The property view displays descriptions of selected properties (in rich text format) at its bottom.
    */
  class MITK_CORE_EXPORT IPropertyDescriptions
  {
  public:
    virtual ~IPropertyDescriptions();

    virtual bool AddDescription(const std::string& propertyName, const std::string& description, bool overwrite = false) = 0;
    virtual std::string GetDescription(const std::string& propertyName) const = 0;
    virtual bool HasDescription(const std::string& propertyName) const = 0;
    virtual void RemoveAllDescriptions() = 0;
    virtual void RemoveDescription(const std::string& propertyName) = 0;
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::IPropertyDescriptions, "org.mitk.IPropertyDescriptions")

#endif
