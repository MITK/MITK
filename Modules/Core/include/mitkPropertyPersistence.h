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

    typedef IPropertyPersistence::InfoMapType InfoMapType;

    bool AddInfo(const std::string& propertyName, PropertyPersistenceInfo::Pointer info, bool overwrite) override;
    InfoMapType GetInfos(const std::string& propertyName) override;
    InfoMapType GetInfosByKey(const std::string& persistenceKey) override;
    PropertyPersistenceInfo::Pointer GetInfo(const std::string& propertyName, const MimeTypeNameType& mime, bool allowWildCard) override;
    bool HasInfos(const std::string& propertyName) override;
    void RemoveAllInfos() override;
    void RemoveInfos(const std::string& propertyName) override;
    void RemoveInfos(const std::string& propertyName, const MimeTypeNameType& mime) override;

  private:
    typedef std::multimap<const std::string, PropertyPersistenceInfo::Pointer> InfoMap;
    typedef InfoMap::const_iterator InfoMapConstIterator;
    typedef InfoMap::iterator InfoMapIterator;

    PropertyPersistence(const PropertyPersistence&);
    PropertyPersistence& operator=(const PropertyPersistence&);

    InfoMap m_Infos;
  };

  /**Creates an unmanaged (!) instance of PropertyPersistence for testing purposes.*/
  MITKCORE_EXPORT IPropertyPersistence* CreateTestInstancePropertyPersistence();
}

#endif
