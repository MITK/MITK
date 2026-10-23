/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPropertyMigration.h>

#include <mitkPropertyList.h>

#include <utility>

mitk::PropertyMigration::PropertyMigration()
{
}

mitk::PropertyMigration::~PropertyMigration()
{
}

void mitk::PropertyMigration::InternalAddMigration(const std::string &legacyKey,
                                                   std::function<bool(const BaseData *)> dataTypeMatcher,
                                                   MigrationFunction migration)
{
  m_Rules.push_back({ legacyKey, std::move(dataTypeMatcher), std::move(migration) });
}

void mitk::PropertyMigration::Migrate(const BaseData *data, PropertyList &properties) const
{
  for (const auto &rule : m_Rules)
  {
    const auto *property = properties.GetProperty(rule.LegacyKey);

    if (property == nullptr || !rule.DataTypeMatcher(data))
      continue;

    // A copy, since a migration may assign a converted value to the property
    // object in the list.
    const BaseProperty::ConstPointer legacyProperty = property->Clone().GetPointer();

    if (rule.Migration)
      rule.Migration(data, *legacyProperty, properties);

    const auto *remainingProperty = properties.GetProperty(rule.LegacyKey);

    if (remainingProperty != nullptr && *remainingProperty == *legacyProperty)
      properties.DeleteProperty(rule.LegacyKey);
  }
}

mitk::IPropertyMigration *mitk::CreateTestInstancePropertyMigration()
{
  return new PropertyMigration();
}
