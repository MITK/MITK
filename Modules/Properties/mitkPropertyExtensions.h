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

#ifndef mitkPropertyExtensions_h
#define mitkPropertyExtensions_h

#include <PropertiesExports.h>
#include <usServiceInterface.h>

namespace mitk
{
  class PropertyExtension;

  class Properties_EXPORT PropertyExtensions
  {
  public:
    virtual ~PropertyExtensions();

    virtual bool AddExtension(const std::string& propertyName, PropertyExtension* extension, bool overwrite = false) = 0;
    virtual PropertyExtension* GetExtension(const std::string& propertyName) const = 0;
    virtual bool HasExtension(const std::string& propertyName) const = 0;
    virtual void RemoveAllExtensions() = 0;
    virtual void RemoveExtension(const std::string& propertyName) = 0;
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::PropertyExtensions, "org.mitk.services.PropertyExtensions")

#endif
