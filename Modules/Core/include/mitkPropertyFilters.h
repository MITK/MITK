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

#ifndef mitkPropertyFilters_h
#define mitkPropertyFilters_h

#include <map>
#include <mitkIPropertyFilters.h>

namespace mitk
{
  class PropertyFilters : public IPropertyFilters
  {
  public:
    PropertyFilters();
    ~PropertyFilters() override;

    bool AddFilter(const PropertyFilter &filter, const std::string &className, bool overwrite) override;
    std::map<std::string, BaseProperty::Pointer> ApplyFilter(
      const std::map<std::string, BaseProperty::Pointer> &propertyMap, const std::string &className) const override;
    PropertyFilter GetFilter(const std::string &className) const override;
    bool HasFilter(const std::string &className) const override;
    void RemoveAllFilters() override;
    void RemoveFilter(const std::string &className) override;

  private:
    PropertyFilters(const PropertyFilters &);
    PropertyFilters &operator=(const PropertyFilters &);

    std::map<std::string, PropertyFilter> m_Filters;
  };
}

#endif
