/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
