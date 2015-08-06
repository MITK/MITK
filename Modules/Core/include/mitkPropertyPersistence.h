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

#ifndef mitkPropertyPersistence_h
#define mitkPropertyPersistence_h

#include <mitkIPropertyPersistence.h>
#include <map>

namespace mitk
{
  class PropertyPersistence : public IPropertyPersistence
  {
  public:
    PropertyPersistence();
    ~PropertyPersistence();

    bool AddInfo(const std::string& propertyName, PropertyPersistenceInfo::Pointer info, bool overwrite) override;
    PropertyPersistenceInfo::Pointer GetInfo(const std::string& propertyName) override;
    bool HasInfo(const std::string& propertyName) override;
    void RemoveAllInfos() override;
    void RemoveInfo(const std::string& propertyName) override;

  private:
    typedef std::map<std::string, PropertyPersistenceInfo::Pointer> InfoMap;
    typedef InfoMap::const_iterator InfoMapConstIterator;
    typedef InfoMap::iterator InfoMapIterator;

    PropertyPersistence(const PropertyPersistence&);
    PropertyPersistence& operator=(const PropertyPersistence&);

    InfoMap m_Infos;
  };
}

#endif
