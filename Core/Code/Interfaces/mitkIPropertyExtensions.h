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

#ifndef mitkIPropertyExtensions_h
#define mitkIPropertyExtensions_h

#include <MitkExports.h>
#include <usServiceInterface.h>

namespace mitk
{
  class PropertyExtension;

  /** \brief Interface of property extensions service.
    *
    * This service allows you to set an extension for a certain property name.
    * An extension is a class that derives from mitk::PropertyExtension.
    * Use extensions to attach useful metadata to your properties, e.g. the allowed range of values.
    * Note that you have to extend the property view if you want it to respect your custom metadata.
    */
  class MITK_CORE_EXPORT IPropertyExtensions
  {
  public:
    virtual ~IPropertyExtensions();

    virtual bool AddExtension(const std::string& propertyName, PropertyExtension* extension, bool overwrite = false) = 0;
    virtual PropertyExtension* GetExtension(const std::string& propertyName) const = 0;
    virtual bool HasExtension(const std::string& propertyName) const = 0;
    virtual void RemoveAllExtensions() = 0;
    virtual void RemoveExtension(const std::string& propertyName) = 0;
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::IPropertyExtensions, "org.mitk.IPropertyExtensions")

#endif
