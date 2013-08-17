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

#include <mitkIPropertyExtensions.h>
#include <map>

namespace mitk
{
  class PropertyExtensions : public IPropertyExtensions
  {
  public:
    PropertyExtensions();
    ~PropertyExtensions();

    bool AddExtension(const std::string& propertyName, PropertyExtension* extension, bool overwrite);
    PropertyExtension* GetExtension(const std::string& propertyName) const;
    bool HasExtension(const std::string& propertyName) const;
    void RemoveAllExtensions();
    void RemoveExtension(const std::string& propertyName);

  private:
    PropertyExtensions(const PropertyExtensions&);
    PropertyExtensions& operator=(const PropertyExtensions&);

    std::map<std::string, PropertyExtension*> m_Extensions;
  };
}

#endif
