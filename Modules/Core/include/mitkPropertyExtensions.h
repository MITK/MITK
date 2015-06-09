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

    bool AddExtension(const std::string& propertyName, PropertyExtension::Pointer extension, const std::string& className, bool overwrite) override;
    PropertyExtension::Pointer GetExtension(const std::string& propertyName, const std::string& className) override;
    bool HasExtension(const std::string& propertyName, const std::string& className) override;
    void RemoveAllExtensions(const std::string& className) override;
    void RemoveExtension(const std::string& propertyName, const std::string& className) override;

  private:
    typedef std::map<std::string, PropertyExtension::Pointer> ExtensionMap;
    typedef ExtensionMap::const_iterator ExtensionMapConstIterator;
    typedef ExtensionMap::iterator ExtensionMapIterator;

    PropertyExtensions(const PropertyExtensions&);
    PropertyExtensions& operator=(const PropertyExtensions&);

    std::map<std::string, ExtensionMap> m_Extensions;
  };
}

#endif
