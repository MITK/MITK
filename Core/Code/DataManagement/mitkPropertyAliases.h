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

#ifndef mitkPropertyAliases_h
#define mitkPropertyAliases_h

#include <mitkIPropertyAliases.h>
#include <map>

namespace mitk
{
  class PropertyAliases : public IPropertyAliases
  {
  public:
    PropertyAliases();
    ~PropertyAliases();

    bool AddAlias(const std::string& propertyName, const std::string& alias, const std::string& className);
    std::vector<std::string> GetAliases(const std::string& propertyName, const std::string& className);
    std::string GetPropertyName(const std::string& alias, const std::string& className);
    bool HasAliases(const std::string& propertyName, const std::string& className);
    void RemoveAlias(const std::string& propertyName, const std::string& alias, const std::string& className);
    void RemoveAliases(const std::string& propertyName, const std::string& className);
    void RemoveAllAliases(const std::string& className);

  private:
    typedef std::map<std::string, std::vector<std::string> > AliasesMap;
    typedef AliasesMap::const_iterator AliasesMapConstIterator;
    typedef AliasesMap::iterator AliasesMapIterator;

    PropertyAliases(const PropertyAliases&);
    PropertyAliases& operator=(const PropertyAliases&);

    std::map<std::string, AliasesMap> m_Aliases;
  };
}

#endif
