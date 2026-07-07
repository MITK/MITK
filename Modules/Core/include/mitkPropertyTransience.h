/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyTransience_h
#define mitkPropertyTransience_h

#include <mitkIPropertyTransience.h>

#include <utility>
#include <vector>

namespace mitk
{
  /** \brief Implementation of the IPropertyTransience service interface.
   *
   * Holds a list of (property name, data-type matcher) rules and reports a
   * property as transient if any rule for that name matches the queried data.
   *
   * \sa IPropertyTransience
   */
  class PropertyTransience : public IPropertyTransience
  {
  public:
    PropertyTransience();
    ~PropertyTransience() override;

    PropertyTransience(const PropertyTransience &) = delete;
    PropertyTransience &operator=(const PropertyTransience &) = delete;

    bool IsTransient(const BaseData *data, const std::string &propertyName) const override;

  protected:
    void InternalAddTransient(const std::string &propertyName,
                              std::function<bool(const BaseData *)> dataTypeMatcher) override;

  private:
    std::vector<std::pair<std::string, std::function<bool(const BaseData *)>>> m_Matchers;
  };

  /** \brief Creates an unmanaged (!) instance of PropertyTransience for testing purposes.
   *
   * The caller is responsible for managing the lifetime of the returned instance.
   *
   * \return A raw pointer to a new PropertyTransience instance.
   */
  MITKCORE_EXPORT IPropertyTransience *CreateTestInstancePropertyTransience();
}

#endif
