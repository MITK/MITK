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
#include <mitkDataStorageReference.h>
#include <mitkStandaloneDataStorage.h>

class mitkDataStorageReferenceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDataStorageReferenceTestSuite);
  MITK_TEST(DefaultConstruction_Invalid);
  MITK_TEST(ParameterizedConstruction_Valid);
  MITK_TEST(GetSetLabel_Success);
  MITK_TEST(GetSetStorage_Success);
  MITK_TEST(IsDefault_ImmutableAfterConstruction);
  MITK_TEST(Equality_SameStorage);
  MITK_TEST(Equality_DifferentStorage);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
  }

  void tearDown() override
  {
  }

  void DefaultConstruction_Invalid()
  {
    mitk::DataStorageReference info;

    CPPUNIT_ASSERT_MESSAGE("Default constructed DataStorageReference should be invalid",
                           !info.IsValid());
    CPPUNIT_ASSERT_MESSAGE("Default label should be empty",
                           info.GetLabel().empty());
    CPPUNIT_ASSERT_MESSAGE("Default IsDefault should be false",
                           !info.IsDefault());
  }

  void ParameterizedConstruction_Valid()
  {
    auto storage = mitk::StandaloneDataStorage::New();
    mitk::DataStorageReference info("Test Label", storage, true);

    CPPUNIT_ASSERT_MESSAGE("Parameterized DataStorageReference should be valid",
                           info.IsValid());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Label should match",
                                 std::string("Test Label"), info.GetLabel());
    CPPUNIT_ASSERT_MESSAGE("Storage should match",
                           info.GetStorage().GetPointer() == storage.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("IsDefault should be true",
                           info.IsDefault());
  }

  void GetSetLabel_Success()
  {
    mitk::DataStorageReference info;
    info.SetLabel("Initial Label");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Label should be set",
                                 std::string("Initial Label"), info.GetLabel());

    info.SetLabel("Changed Label");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Label should be changed",
                                 std::string("Changed Label"), info.GetLabel());
  }

  void GetSetStorage_Success()
  {
    auto storage1 = mitk::StandaloneDataStorage::New();
    auto storage2 = mitk::StandaloneDataStorage::New();

    mitk::DataStorageReference info;
    CPPUNIT_ASSERT_MESSAGE("Initial storage should be null",
                           info.GetStorage().IsNull());

    info.SetStorage(storage1);
    CPPUNIT_ASSERT_MESSAGE("Storage should be set",
                           info.GetStorage().GetPointer() == storage1.GetPointer());

    info.SetStorage(storage2);
    CPPUNIT_ASSERT_MESSAGE("Storage should be changed",
                           info.GetStorage().GetPointer() == storage2.GetPointer());
  }

  void IsDefault_ImmutableAfterConstruction()
  {
    // IsDefault can only be set at construction time
    mitk::DataStorageReference infoDefault("Default", nullptr, true);
    mitk::DataStorageReference infoNonDefault("NonDefault", nullptr, false);

    CPPUNIT_ASSERT_MESSAGE("IsDefault should be true for default-constructed info",
                           infoDefault.IsDefault());
    CPPUNIT_ASSERT_MESSAGE("IsDefault should be false for non-default info",
                           !infoNonDefault.IsDefault());

    // There should be no setter for IsDefault - the value is immutable
    // This is verified by the fact that there's no SetIsDefault method
  }

  void Equality_SameStorage()
  {
    auto storage = mitk::StandaloneDataStorage::New();
    mitk::DataStorageReference info1("Label1", storage, true);
    mitk::DataStorageReference info2("Label2", storage, false);

    // Equality is based on storage pointer, not label or IsDefault
    CPPUNIT_ASSERT_MESSAGE("Two DataStorageReference with same storage should be equal",
                           info1 == info2);
    CPPUNIT_ASSERT_MESSAGE("Inequality should return false for same storage",
                           !(info1 != info2));
  }

  void Equality_DifferentStorage()
  {
    auto storage1 = mitk::StandaloneDataStorage::New();
    auto storage2 = mitk::StandaloneDataStorage::New();
    mitk::DataStorageReference info1("Label", storage1, true);
    mitk::DataStorageReference info2("Label", storage2, true);

    // Different storage means different info, even with same label and IsDefault
    CPPUNIT_ASSERT_MESSAGE("Two DataStorageReference with different storage should not be equal",
                           info1 != info2);
    CPPUNIT_ASSERT_MESSAGE("Equality should return false for different storage",
                           !(info1 == info2));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDataStorageReference)
