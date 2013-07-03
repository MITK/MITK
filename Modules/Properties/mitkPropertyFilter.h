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

#ifndef mitkPropertyFilter_h
#define mitkPropertyFilter_h

#include <mitkBaseProperty.h>
#include <PropertiesExports.h>

namespace mitk
{
  class Properties_EXPORT PropertyFilter
  {
  public:
    enum List
    {
      Blacklist,
      Whitelist
    };

    PropertyFilter();
    ~PropertyFilter();

    PropertyFilter(const PropertyFilter& other);
    PropertyFilter& operator=(PropertyFilter other);

    void AddEntry(const std::string& propertyName, List list);
    std::map<std::string, BaseProperty::Pointer> Apply(const std::map<std::string, BaseProperty::Pointer>& propertyMap) const;
    bool HasEntry(const std::string& propertyName, List list) const;
    bool IsEmpty() const;
    void RemoveAllEntries(List list);
    void RemoveEntry(const std::string& propertyName, List list);

  private:
    class Impl;
    Impl* m_Impl;
  };
}

#endif
