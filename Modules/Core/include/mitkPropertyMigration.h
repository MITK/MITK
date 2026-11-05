/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyMigration_h
#define mitkPropertyMigration_h

#include <mitkIPropertyMigration.h>

#include <vector>

namespace mitk
{
  /** \brief Implementation of the IPropertyMigration service interface.
   *
   * Holds a list of (legacy key, data-type matcher, migration) rules and
   * applies them in registration order.
   *
   * \sa IPropertyMigration
   */
  class PropertyMigration : public IPropertyMigration
  {
  public:
    PropertyMigration();
    ~PropertyMigration() override;

    PropertyMigration(const PropertyMigration &) = delete;
    PropertyMigration &operator=(const PropertyMigration &) = delete;

    void Migrate(const BaseData *data, PropertyList &properties) const override;

  protected:
    void InternalAddMigration(const std::string &legacyKey,
                              std::function<bool(const BaseData *)> dataTypeMatcher,
                              MigrationFunction migration) override;

  private:
    struct Rule
    {
      std::string LegacyKey;
      std::function<bool(const BaseData *)> DataTypeMatcher;
      MigrationFunction Migration;
    };

    std::vector<Rule> m_Rules;
  };

  /** \brief Creates an unmanaged (!) instance of PropertyMigration for testing purposes.
   *
   * The caller is responsible for managing the lifetime of the returned instance.
   *
   * \return A raw pointer to a new PropertyMigration instance.
   */
  MITKCORE_EXPORT IPropertyMigration *CreateTestInstancePropertyMigration();
}

#endif
