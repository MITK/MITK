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
#include <MitkExports.h>

namespace mitk
{
  /** \brief Consists of blacklist and whitelist entries.
    *
    * Blacklist filtering is less restrictive than whitelist filtering since you can filter out individual property names.
    * If whitelist entries were added, all other property names are filtered out.
    * In addition, whitelist entries are reduced by blacklist entries.
    */
  class MITK_CORE_EXPORT PropertyFilter
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
