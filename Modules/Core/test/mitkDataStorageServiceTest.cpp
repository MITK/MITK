/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// MITK includes
#include <mitkCoreServices.h>
#include <mitkDataStorageService.h>
#include <mitkIDataStorageService.h>
#include <mitkStandaloneDataStorage.h>

/**
 * \brief Unit tests for IDataStorageService interface and DataStorageService implementation.
 */
class mitkDataStorageServiceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDataStorageServiceTestSuite);
  MITK_TEST(GetDataStorageService_WithoutImplementation_ReturnsNull);
  MITK_TEST(CoreServicePointer_WithNullService_HandlesSafely);
  MITK_TEST(Constructor_CreatesDefaultStorage);
  MITK_TEST(GetDefaultDataStorage_ReturnsValidStorage);
  MITK_TEST(GetActiveDataStorage_InitiallyReturnsDefault);
  MITK_TEST(GetActiveDataStorageReference_InitiallyReturnsDefault);
  MITK_TEST(CreateDataStorage_CreatesNewStorage);
  MITK_TEST(CreateDataStorage_WithDuplicateLabel_ReturnsInvalid);
  MITK_TEST(CreateDataStorage_WithEmptyLabel_ReturnsInvalid);
  MITK_TEST(AddDataStorage_AddsExistingStorage);
  MITK_TEST(AddDataStorage_WithDuplicateLabel_ReturnsFalse);
  MITK_TEST(SetActiveDataStorage_SwitchesStorage);
  MITK_TEST(SetActiveDataStorage_WithInvalidLabel_ReturnsFalse);
  MITK_TEST(GetDataStorageReference_ReturnsCorrectInfo);
  MITK_TEST(GetLabel_ReturnsCorrectLabel);
  MITK_TEST(GetAllLabels_ReturnsAllLabels);
  MITK_TEST(GetAllDataStorages_ReturnsAllStorages);
  MITK_TEST(HasDataStorage_ReturnsCorrectResult);
  MITK_TEST(RemoveDataStorage_RemovesStorage);
  MITK_TEST(RemoveDataStorage_CannotRemoveDefault);
  MITK_TEST(RemoveDataStorage_ResetsActiveIfRemoved);
  CPPUNIT_TEST_SUITE_END();

private:
  std::unique_ptr<mitk::DataStorageService> m_Service;

public:
  void setUp() override
  {
    m_Service = std::make_unique<mitk::DataStorageService>();
  }

  void tearDown() override
  {
    m_Service.reset();
  }

  void GetDataStorageService_WithoutImplementation_ReturnsNull()
  {
    // When no plugin has registered an IDataStorageService implementation,
    // GetDataStorageService should return nullptr
    auto* service = mitk::CoreServices::GetDataStorageService();

    CPPUNIT_ASSERT_MESSAGE(
      "GetDataStorageService should return nullptr when no implementation is registered",
      service == nullptr);
  }

  void CoreServicePointer_WithNullService_HandlesSafely()
  {
    // CoreServicePointer should handle null services gracefully
    mitk::CoreServicePointer<mitk::IDataStorageService> servicePtr(
      mitk::CoreServices::GetDataStorageService());

    // The pointer should evaluate to false for null service
    CPPUNIT_ASSERT_MESSAGE(
      "CoreServicePointer should evaluate to false for null service",
      !servicePtr);

    // Get() should return nullptr
    CPPUNIT_ASSERT_MESSAGE(
      "CoreServicePointer::Get() should return nullptr for null service",
      servicePtr.Get() == nullptr);

    // Destructor should not crash when service is null
    // (verified by test completing without exception)
  }

  void Constructor_CreatesDefaultStorage()
  {
    CPPUNIT_ASSERT_MESSAGE(
      "Service should have default storage after construction",
      m_Service->HasDataStorage(mitk::DataStorageService::DEFAULT_LABEL));
  }

  void GetDefaultDataStorage_ReturnsValidStorage()
  {
    auto storage = m_Service->GetDefaultDataStorage();
    CPPUNIT_ASSERT_MESSAGE(
      "GetDefaultDataStorage should return a valid DataStorage",
      storage.IsNotNull());
  }

  void GetActiveDataStorage_InitiallyReturnsDefault()
  {
    auto active = m_Service->GetActiveDataStorage();
    auto defaultStorage = m_Service->GetDefaultDataStorage();

    CPPUNIT_ASSERT_MESSAGE(
      "GetActiveDataStorage should initially return the default storage",
      active.GetPointer() == defaultStorage.GetPointer());
  }

  void GetActiveDataStorageReference_InitiallyReturnsDefault()
  {
    auto info = m_Service->GetActiveDataStorageReference();

    CPPUNIT_ASSERT_MESSAGE(
      "Active storage info should be valid",
      info.IsValid());

    CPPUNIT_ASSERT_MESSAGE(
      "Active storage info should be marked as default",
      info.IsDefault());

    CPPUNIT_ASSERT_MESSAGE(
      "Active storage label should be the default label",
      info.GetLabel() == mitk::DataStorageService::DEFAULT_LABEL);
  }

  void CreateDataStorage_CreatesNewStorage()
  {
    auto info = m_Service->CreateDataStorage("TestStorage");

    CPPUNIT_ASSERT_MESSAGE(
      "CreateDataStorage should return valid info",
      info.IsValid());

    CPPUNIT_ASSERT_MESSAGE(
      "Created storage should have correct label",
      info.GetLabel() == "TestStorage");

    CPPUNIT_ASSERT_MESSAGE(
      "Created storage should not be marked as default",
      !info.IsDefault());

    CPPUNIT_ASSERT_MESSAGE(
      "Service should have the created storage",
      m_Service->HasDataStorage("TestStorage"));
  }

  void CreateDataStorage_WithDuplicateLabel_ReturnsInvalid()
  {
    m_Service->CreateDataStorage("Duplicate");
    auto info = m_Service->CreateDataStorage("Duplicate");

    CPPUNIT_ASSERT_MESSAGE(
      "CreateDataStorage with duplicate label should return invalid info",
      !info.IsValid());
  }

  void CreateDataStorage_WithEmptyLabel_ReturnsInvalid()
  {
    auto info = m_Service->CreateDataStorage("");

    CPPUNIT_ASSERT_MESSAGE(
      "CreateDataStorage with empty label should return invalid info",
      !info.IsValid());
  }

  void AddDataStorage_AddsExistingStorage()
  {
    auto storage = mitk::StandaloneDataStorage::New();
    bool added = m_Service->AddDataStorage("External", storage.GetPointer());

    CPPUNIT_ASSERT_MESSAGE(
      "AddDataStorage should return true for valid input",
      added);

    CPPUNIT_ASSERT_MESSAGE(
      "Service should have the added storage",
      m_Service->HasDataStorage("External"));

    auto info = m_Service->GetDataStorageReference("External");
    CPPUNIT_ASSERT_MESSAGE(
      "GetDataStorageReference should return the added storage",
      info.has_value() && info->GetStorage().GetPointer() == storage.GetPointer());
  }

  void AddDataStorage_WithDuplicateLabel_ReturnsFalse()
  {
    auto storage1 = mitk::StandaloneDataStorage::New();
    auto storage2 = mitk::StandaloneDataStorage::New();

    m_Service->AddDataStorage("Same", storage1.GetPointer());
    bool added = m_Service->AddDataStorage("Same", storage2.GetPointer());

    CPPUNIT_ASSERT_MESSAGE(
      "AddDataStorage with duplicate label should return false",
      !added);
  }

  void SetActiveDataStorage_SwitchesStorage()
  {
    auto info = m_Service->CreateDataStorage("NewActive");
    bool set = m_Service->SetActiveDataStorage("NewActive");

    CPPUNIT_ASSERT_MESSAGE(
      "SetActiveDataStorage should return true for valid label",
      set);

    auto active = m_Service->GetActiveDataStorage();
    CPPUNIT_ASSERT_MESSAGE(
      "GetActiveDataStorage should return the newly set storage",
      active.GetPointer() == info.GetStorage().GetPointer());

    auto activeInfo = m_Service->GetActiveDataStorageReference();
    CPPUNIT_ASSERT_MESSAGE(
      "GetActiveDataStorageReference should return info for new active storage",
      activeInfo.GetLabel() == "NewActive");
  }

  void SetActiveDataStorage_WithInvalidLabel_ReturnsFalse()
  {
    bool set = m_Service->SetActiveDataStorage("NonExistent");

    CPPUNIT_ASSERT_MESSAGE(
      "SetActiveDataStorage with invalid label should return false",
      !set);

    // Active should still be default
    auto activeInfo = m_Service->GetActiveDataStorageReference();
    CPPUNIT_ASSERT_MESSAGE(
      "Active storage should still be default after failed set",
      activeInfo.IsDefault());
  }

  void GetDataStorageReference_ReturnsCorrectInfo()
  {
    m_Service->CreateDataStorage("Other1");
    m_Service->CreateDataStorage("FindMe");
    m_Service->CreateDataStorage("Other2");

    auto info = m_Service->GetDataStorageReference("FindMe");
    CPPUNIT_ASSERT_MESSAGE(
      "GetDataStorageReference should return value for existing storage",
      info.has_value());

    CPPUNIT_ASSERT_MESSAGE(
      "GetDataStorageReference should return correct label",
      info->GetLabel() == "FindMe");

    auto notFound = m_Service->GetDataStorageReference("NotThere");
    CPPUNIT_ASSERT_MESSAGE(
      "GetDataStorageReference should return nullopt for non-existent storage",
      !notFound.has_value());
  }

  void GetLabel_ReturnsCorrectLabel()
  {
    m_Service->CreateDataStorage("Other1");
    auto info = m_Service->CreateDataStorage("LabelTest");
    m_Service->CreateDataStorage("Other2");
    auto storage = info.GetStorage();

    auto label = m_Service->GetLabel(storage.GetPointer());
    CPPUNIT_ASSERT_MESSAGE(
      "GetLabel should return value for known storage",
      label.has_value());

    CPPUNIT_ASSERT_MESSAGE(
      "GetLabel should return correct label",
      label.value() == "LabelTest");

    auto unknownStorage = mitk::StandaloneDataStorage::New();
    auto notFound = m_Service->GetLabel(unknownStorage.GetPointer());
    CPPUNIT_ASSERT_MESSAGE(
      "GetLabel should return nullopt for unknown storage",
      !notFound.has_value());
  }

  void GetAllLabels_ReturnsAllLabels()
  {
    m_Service->CreateDataStorage("Storage1");
    m_Service->CreateDataStorage("Storage2");

    auto labels = m_Service->GetAllLabels();

    CPPUNIT_ASSERT_MESSAGE(
      "GetAllLabels should return 3 labels (default + 2 created)",
      labels.size() == 3);

    // Check that default is first
    CPPUNIT_ASSERT_MESSAGE(
      "First label should be the default",
      labels[0] == mitk::DataStorageService::DEFAULT_LABEL);
  }

  void GetAllDataStorages_ReturnsAllStorages()
  {
    m_Service->CreateDataStorage("All1");
    m_Service->CreateDataStorage("All2");

    auto storages = m_Service->GetAllDataStorages();

    CPPUNIT_ASSERT_MESSAGE(
      "GetAllDataStorages should return 3 storages (default + 2 created)",
      storages.size() == 3);

    // Check that default is first
    CPPUNIT_ASSERT_MESSAGE(
      "First storage should be the default",
      storages[0].IsDefault());
  }

  void HasDataStorage_ReturnsCorrectResult()
  {
    m_Service->CreateDataStorage("Other1");
    m_Service->CreateDataStorage("Other2");
    m_Service->CreateDataStorage("Exists");

    CPPUNIT_ASSERT_MESSAGE(
      "HasDataStorage should return true for default",
      m_Service->HasDataStorage(mitk::DataStorageService::DEFAULT_LABEL));

    CPPUNIT_ASSERT_MESSAGE(
      "HasDataStorage should return false for non-existent",
      !m_Service->HasDataStorage("DoesNotExist"));

    CPPUNIT_ASSERT_MESSAGE(
      "HasDataStorage should return true for created storage",
      m_Service->HasDataStorage("Exists"));
  }

  void RemoveDataStorage_RemovesStorage()
  {
    m_Service->CreateDataStorage("Exists");
    m_Service->CreateDataStorage("ToRemove");
    CPPUNIT_ASSERT(m_Service->HasDataStorage("ToRemove"));

    bool removed = m_Service->RemoveDataStorage("ToRemove");

    CPPUNIT_ASSERT_MESSAGE(
      "RemoveDataStorage should return true",
      removed);

    CPPUNIT_ASSERT_MESSAGE(
      "Storage should no longer exist",
      !m_Service->HasDataStorage("ToRemove"));

    CPPUNIT_ASSERT_MESSAGE(
      "Other storage should not be removed",
      m_Service->HasDataStorage("Exists"));

  }

  void RemoveDataStorage_CannotRemoveDefault()
  {
    bool removed = m_Service->RemoveDataStorage(mitk::DataStorageService::DEFAULT_LABEL);

    CPPUNIT_ASSERT_MESSAGE(
      "RemoveDataStorage should return false for default storage",
      !removed);

    CPPUNIT_ASSERT_MESSAGE(
      "Default storage should still exist",
      m_Service->HasDataStorage(mitk::DataStorageService::DEFAULT_LABEL));
  }

  void RemoveDataStorage_ResetsActiveIfRemoved()
  {
    m_Service->CreateDataStorage("WillBeRemoved");
    m_Service->SetActiveDataStorage("WillBeRemoved");

    // Verify it's active
    auto activeInfo = m_Service->GetActiveDataStorageReference();
    CPPUNIT_ASSERT(activeInfo.GetLabel() == "WillBeRemoved");

    // Remove it
    m_Service->RemoveDataStorage("WillBeRemoved");

    // Active should reset to default
    activeInfo = m_Service->GetActiveDataStorageReference();
    CPPUNIT_ASSERT_MESSAGE(
      "Active storage should reset to default after removal",
      activeInfo.IsDefault());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDataStorageService)
