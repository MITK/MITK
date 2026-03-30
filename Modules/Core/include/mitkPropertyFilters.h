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
  /** \brief Implementation of the IPropertyFilters service interface.
   *
   * Manages PropertyFilter instances that can be associated with specific data
   * node class names. Applying a filter first applies the global filter (empty
   * class name), then the class-specific filter.
   *
   * \sa IPropertyFilters
   */
  class PropertyFilters : public IPropertyFilters
  {
  public:
    PropertyFilters();
    ~PropertyFilters() override;

    /** \copydoc IPropertyFilters::AddFilter */
    bool AddFilter(const PropertyFilter &filter, const std::string &className, bool overwrite) override;
    /** \copydoc IPropertyFilters::ApplyFilter */
    std::map<std::string, BaseProperty::Pointer> ApplyFilter(
      const std::map<std::string, BaseProperty::Pointer> &propertyMap, const std::string &className) const override;
    /** \copydoc IPropertyFilters::GetFilter */
    PropertyFilter GetFilter(const std::string &className) const override;
    /** \copydoc IPropertyFilters::HasFilter */
    bool HasFilter(const std::string &className) const override;
    /** \copydoc IPropertyFilters::RemoveAllFilters */
    void RemoveAllFilters() override;
    /** \copydoc IPropertyFilters::RemoveFilter */
    void RemoveFilter(const std::string &className) override;

  private:
    PropertyFilters(const PropertyFilters &);
    PropertyFilters &operator=(const PropertyFilters &);

    std::map<std::string, PropertyFilter> m_Filters;
  };
}

#endif
