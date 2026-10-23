/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkIPropertyMigration.h>
#include <mitkPropertyMigration.h>

#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <mitkSlicedData.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

namespace
{
  // Moves a float property to a new key, the typical rename of a mapper property.
  mitk::IPropertyMigration::MigrationFunction MoveFloatTo(const std::string &key)
  {
    return [key](const mitk::BaseData *, const mitk::BaseProperty &legacyProperty, mitk::PropertyList &properties) {
      const auto *floatProperty = dynamic_cast<const mitk::FloatProperty *>(&legacyProperty);

      if (floatProperty != nullptr)
        properties.SetFloatProperty(key.c_str(), floatProperty->GetValue());
    };
  }
}

class mitkPropertyMigrationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyMigrationTestSuite);

  MITK_TEST(Migrate_MovesValueAndRemovesLegacyKey);
  MITK_TEST(Migrate_Removal);
  MITK_TEST(Migrate_SpecificType);
  MITK_TEST(Migrate_Subclass);
  MITK_TEST(Migrate_NullData);
  MITK_TEST(Migrate_OverwritesExistingReplacement);
  MITK_TEST(Migrate_ConvertsInPlace);
  MITK_TEST(Migrate_FirstMatchingRuleWins);
  MITK_TEST(Migrate_LeavesOtherPropertiesAlone);
  MITK_TEST(ServiceIsRegistered);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_Image;
  mitk::PointSet::Pointer m_PointSet;
  mitk::PropertyList::Pointer m_Properties;
  mitk::IPropertyMigration *m_Service;

public:
  void setUp() override
  {
    m_Image = mitk::Image::New();
    m_PointSet = mitk::PointSet::New();
    m_Properties = mitk::PropertyList::New();
    m_Properties->SetFloatProperty("legacy.width", 4.0f);
    m_Service = mitk::CreateTestInstancePropertyMigration();
  }

  void tearDown() override
  {
    m_Image = nullptr;
    m_PointSet = nullptr;
    m_Properties = nullptr;
    delete m_Service;
  }

  void Migrate_MovesValueAndRemovesLegacyKey()
  {
    m_Service->AddMigration<mitk::BaseData>("legacy.width", MoveFloatTo("width"));
    m_Service->Migrate(m_Image, *m_Properties);

    float width = 0.0f;
    CPPUNIT_ASSERT_MESSAGE("Replacement is written", m_Properties->GetFloatProperty("width", width));
    CPPUNIT_ASSERT_EQUAL(4.0f, width);
    CPPUNIT_ASSERT_MESSAGE("Legacy key is removed", m_Properties->GetProperty("legacy.width") == nullptr);
  }

  void Migrate_Removal()
  {
    m_Service->AddRemoval<mitk::BaseData>("legacy.width");
    m_Service->Migrate(m_Image, *m_Properties);

    CPPUNIT_ASSERT_MESSAGE("Legacy key is removed", m_Properties->GetProperty("legacy.width") == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Nothing else is left", m_Properties->IsEmpty());
  }

  void Migrate_SpecificType()
  {
    m_Service->AddRemoval<mitk::Image>("legacy.width");

    m_Service->Migrate(m_PointSet, *m_Properties);
    CPPUNIT_ASSERT_MESSAGE("Rule does not apply to another type", m_Properties->GetProperty("legacy.width") != nullptr);

    m_Service->Migrate(m_Image, *m_Properties);
    CPPUNIT_ASSERT_MESSAGE("Rule applies to its type", m_Properties->GetProperty("legacy.width") == nullptr);
  }

  void Migrate_Subclass()
  {
    // Image is-a SlicedData, so a rule registered for the base type must apply to the subclass.
    m_Service->AddRemoval<mitk::SlicedData>("legacy.width");
    m_Service->Migrate(m_Image, *m_Properties);

    CPPUNIT_ASSERT_MESSAGE("Base-type rule applies to subclass instance", m_Properties->GetProperty("legacy.width") == nullptr);
  }

  void Migrate_NullData()
  {
    m_Properties->SetFloatProperty("legacy.any", 1.0f);
    m_Service->AddRemoval<mitk::Image>("legacy.width");
    m_Service->AddRemoval<mitk::BaseData>("legacy.any");
    m_Service->Migrate(nullptr, *m_Properties);

    CPPUNIT_ASSERT_MESSAGE("Specific-type rule does not apply to null data", m_Properties->GetProperty("legacy.width") != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Any-type rule applies to null data", m_Properties->GetProperty("legacy.any") == nullptr);
  }

  void Migrate_OverwritesExistingReplacement()
  {
    // Readers may have added the new key with its default value before migrating.
    m_Properties->SetFloatProperty("width", 2.0f);
    m_Service->AddMigration<mitk::BaseData>("legacy.width", MoveFloatTo("width"));
    m_Service->Migrate(m_Image, *m_Properties);

    float width = 0.0f;
    m_Properties->GetFloatProperty("width", width);
    CPPUNIT_ASSERT_EQUAL(4.0f, width);
  }

  void Migrate_ConvertsInPlace()
  {
    m_Service->AddMigration<mitk::BaseData>("legacy.width", [](const mitk::BaseData *, const mitk::BaseProperty &, mitk::PropertyList &properties) {
      properties.SetFloatProperty("legacy.width", 1.0f);
    });
    m_Service->Migrate(m_Image, *m_Properties);

    float width = 0.0f;
    CPPUNIT_ASSERT_MESSAGE("A value changed under the legacy key is kept", m_Properties->GetFloatProperty("legacy.width", width));
    CPPUNIT_ASSERT_EQUAL(1.0f, width);
  }

  void Migrate_FirstMatchingRuleWins()
  {
    m_Service->AddMigration<mitk::PointSet>("legacy.width", MoveFloatTo("point set width"));
    m_Service->AddMigration<mitk::Image>("legacy.width", MoveFloatTo("image width"));
    m_Service->AddMigration<mitk::BaseData>("legacy.width", MoveFloatTo("any width"));
    m_Service->Migrate(m_Image, *m_Properties);

    CPPUNIT_ASSERT_MESSAGE("Non-matching rule is skipped", m_Properties->GetProperty("point set width") == nullptr);
    CPPUNIT_ASSERT_MESSAGE("First matching rule migrates", m_Properties->GetProperty("image width") != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Later rules see no legacy property", m_Properties->GetProperty("any width") == nullptr);
  }

  void Migrate_LeavesOtherPropertiesAlone()
  {
    m_Properties->SetFloatProperty("unrelated", 3.0f);
    m_Service->AddRemoval<mitk::BaseData>("legacy.width");
    m_Service->AddRemoval<mitk::BaseData>("legacy.absent");
    m_Service->Migrate(m_Image, *m_Properties);

    CPPUNIT_ASSERT_MESSAGE("Unregistered property stays", m_Properties->GetProperty("unrelated") != nullptr);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), m_Properties->GetMap()->size());
  }

  void ServiceIsRegistered()
  {
    mitk::CoreServicePointer<mitk::IPropertyMigration> migration(mitk::CoreServices::GetPropertyMigration());
    CPPUNIT_ASSERT_MESSAGE("Core registers the property migration service", migration.Get() != nullptr);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyMigration)
