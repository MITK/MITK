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

#include "mitkIOMimeTypes.h"
#include "mitkPropertyPersistenceInfo.h"
#include "mitkStringProperty.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <limits>

::std::string testSerializeFunction(const mitk::BaseProperty * /*prop*/)
{
  return "testSerialize";
}

mitk::BaseProperty::Pointer testDeserializeFunction(const std::string & /*value*/)
{
  mitk::StringProperty::Pointer result = mitk::StringProperty::New("testSerialize");
  return result.GetPointer();
}

class mitkPropertyPersistenceInfoTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyPersistenceInfoTestSuite);

  MITK_TEST(CheckDefaultInfo);
  MITK_TEST(SetName);
  MITK_TEST(SetNameAndKey);
  MITK_TEST(SetMimeTypeName);
  MITK_TEST(UseRegEx);
  MITK_TEST(UseRegEx2);
  MITK_TEST(UnRegExByName);
  MITK_TEST(UnRegExByKey);
  MITK_TEST(SetDeserializationFunction);
  MITK_TEST(SetSerializationFunction);
  MITK_TEST(serializeByGetValueAsString);
  MITK_TEST(deserializeToStringProperty);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StringProperty::Pointer refProp;
  std::string refSerialization;
  mitk::StringProperty::Pointer refProp_testFunction;
  std::string refSerialization_testFunction;
  mitk::PropertyPersistenceInfo::Pointer info;
  mitk::PropertyPersistenceInfo::Pointer regexInfo;

  std::string nameRegEx;
  std::string keyRegEx;
  std::string nameTemplate;
  std::string keyTemplate;

public:
  void setUp() override
  {
    refSerialization = "my_shiny_test_value";
    refProp = mitk::StringProperty::New("my_shiny_test_value");
    info = mitk::PropertyPersistenceInfo::New();
    refSerialization_testFunction = "testSerialize";
    refProp_testFunction = mitk::StringProperty::New("testSerialize");

    nameRegEx = "name(\\d*)";
    nameTemplate = "$1_name";
    keyRegEx = "key(\\d*)";
    keyTemplate = "newkey_[$1]";

    regexInfo = mitk::PropertyPersistenceInfo::New();
    regexInfo->UseRegEx(nameRegEx, nameTemplate, keyRegEx, keyTemplate);
  }

  void tearDown() override {}
  void CheckDefaultInfo()
  {
    CPPUNIT_ASSERT_MESSAGE(
      "Testing PropertyPersistenceInfo::ANY_MIMETYPE_NAME()",
      mitk::PropertyPersistenceInfo::ANY_MIMETYPE_NAME() == mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".any_type");
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::GetName()", info->GetName() == "");
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::GetKey()", info->GetKey() == "");
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::GetMimeTypeName()",
                           info->GetMimeTypeName() == mitk::PropertyPersistenceInfo::ANY_MIMETYPE_NAME());
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::IsRegEx()", !info->IsRegEx());

    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::IsRegEx()", regexInfo->IsRegEx());

    mitk::BaseProperty::Pointer prop = info->GetDeserializationFunction()(refSerialization);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::GetDeserializationFunction() producing a StringProperty",
                           dynamic_cast<mitk::StringProperty *>(prop.GetPointer()));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing PropertyPersistenceInfo::GetDeserializationFunction() producing a StringProperty with correct value",
      prop->GetValueAsString() == refSerialization);

    std::string value = info->GetSerializationFunction()(refProp);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::GetSerializationFunction()", value == refSerialization);
  }

  void SetName()
  {
    info->SetName("MyName");
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::SetKey() changed name", info->GetName() == "MyName");
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::SetKey() changed key", info->GetKey() == "MyName");

    // test if setter resets to non regex
    regexInfo->SetName("MyName");
    CPPUNIT_ASSERT_MESSAGE("Testing regex reset when using SetName()", !regexInfo->IsRegEx());
  }

  void SetNameAndKey()
  {
    info->SetNameAndKey("MyName", "MyKey");
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::SetNameAndKey() changed name",
                           info->GetName() == "MyName");
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::SetNameAndKey() changed key", info->GetKey() == "MyKey");

    // test if setter resets to non regex
    regexInfo->SetNameAndKey("MyName", "MyKey");
    CPPUNIT_ASSERT_MESSAGE("Testing regex reset when using SetNameAndKey()", !regexInfo->IsRegEx());
  }

  void SetMimeTypeName()
  {
    info->SetMimeTypeName("newMime");
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::SetMimeTypeName()", info->GetMimeTypeName() == "newMime");
  }

  void UseRegEx()
  {
    info->UseRegEx(nameRegEx, nameTemplate);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::IsRegEx()", info->IsRegEx());
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::UseRegEx() changed name", info->GetName() == nameRegEx);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::UseRegEx() changed key", info->GetKey() == nameRegEx);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::UseRegEx() changed name template",
                           info->GetNameTemplate() == nameTemplate);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::UseRegEx() changed key template",
                           info->GetKeyTemplate() == nameTemplate);
  }

  void UseRegEx2()
  {
    info->UseRegEx(nameRegEx, nameTemplate, keyRegEx, keyTemplate);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::IsRegEx()", info->IsRegEx());
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::UseRegEx() changed name", info->GetName() == nameRegEx);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::UseRegEx() changed key", info->GetKey() == keyRegEx);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::UseRegEx() changed name template",
                           info->GetNameTemplate() == nameTemplate);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::UseRegEx() changed key template",
                           info->GetKeyTemplate() == keyTemplate);
  }

  void UnRegExByName()
  {
    mitk::PropertyPersistenceInfo::Pointer newInfo = regexInfo->UnRegExByName("name42");
    CPPUNIT_ASSERT_MESSAGE("Testing IsRegEx() of \"unreg\"ed regexInfo", !newInfo->IsRegEx());
    CPPUNIT_ASSERT_MESSAGE("Testing GetName() of \"unreg\"ed regexInfo", newInfo->GetName() == "name42");
    CPPUNIT_ASSERT_MESSAGE("Testing GetKey() of \"unreg\"ed regexInfo", newInfo->GetKey() == "newkey_[42]");

    newInfo = info->UnRegExByName("name42");
    CPPUNIT_ASSERT_MESSAGE("Testing IsRegEx() of \"unreg\"ed info", !newInfo->IsRegEx());
    CPPUNIT_ASSERT_MESSAGE("Testing GetName() of \"unreg\"ed info", newInfo->GetName() == info->GetName());
    CPPUNIT_ASSERT_MESSAGE("Testing GetKey() of \"unreg\"ed info", newInfo->GetKey() == info->GetKey());
  }

  void UnRegExByKey()
  {
    mitk::PropertyPersistenceInfo::Pointer newInfo = regexInfo->UnRegExByKey("key42");
    CPPUNIT_ASSERT_MESSAGE("Testing IsRegEx() of \"unreg\"ed regexInfo", !newInfo->IsRegEx());
    CPPUNIT_ASSERT_MESSAGE("Testing GetName() of \"unreg\"ed regexInfo", newInfo->GetName() == "42_name");
    CPPUNIT_ASSERT_MESSAGE("Testing GetKey() of \"unreg\"ed regexInfo", newInfo->GetKey() == "key42");

    newInfo = info->UnRegExByKey("key42");
    CPPUNIT_ASSERT_MESSAGE("Testing IsRegEx() of \"unreg\"ed info", !newInfo->IsRegEx());
    CPPUNIT_ASSERT_MESSAGE("Testing GetName() of \"unreg\"ed info", newInfo->GetName() == info->GetName());
    CPPUNIT_ASSERT_MESSAGE("Testing GetKey() of \"unreg\"ed info", newInfo->GetKey() == info->GetKey());
  }

  void SetDeserializationFunction()
  {
    info->SetDeserializationFunction(testDeserializeFunction);
    mitk::BaseProperty::Pointer prop = info->GetDeserializationFunction()(refSerialization);
    CPPUNIT_ASSERT_MESSAGE(
      "Testing PropertyPersistenceInfo::SetDeserializationFunction() producing a StringProperty with correct value",
      prop->GetValueAsString() == refSerialization_testFunction);
  }

  void SetSerializationFunction()
  {
    info->SetSerializationFunction(testSerializeFunction);
    std::string value = info->GetSerializationFunction()(refProp);
    CPPUNIT_ASSERT_MESSAGE("Testing PropertyPersistenceInfo::SetSerializationFunction()",
                           value == refSerialization_testFunction);
  }

  void serializeByGetValueAsString()
  {
    std::string value = mitk::PropertyPersistenceSerialization::serializeByGetValueAsString(refProp);
    CPPUNIT_ASSERT_MESSAGE("Testing serializeByGetValueAsString()", value == refSerialization);
  }

  void deserializeToStringProperty()
  {
    mitk::BaseProperty::Pointer prop =
      mitk::PropertyPersistenceDeserialization::deserializeToStringProperty(refSerialization);
    CPPUNIT_ASSERT_MESSAGE("Testing deserializeToStringProperty() producing a StringProperty",
                           dynamic_cast<mitk::StringProperty *>(prop.GetPointer()));
    CPPUNIT_ASSERT_MESSAGE("Testing deserializeToStringProperty() producing a StringProperty with correct value",
                           prop->GetValueAsString() == refSerialization);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyPersistenceInfo)
