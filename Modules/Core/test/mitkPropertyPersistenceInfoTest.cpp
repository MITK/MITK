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

#include "mitkPropertyPersistenceInfo.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include "mitkStringProperty.h"
#include "mitkIOMimeTypes.h"

#include <limits>

::std::string testSerializeFunction(const mitk::BaseProperty* prop)
{
  return "testSerialize";
}

mitk::BaseProperty::Pointer testDeserializeFunction(const std::string& value)
{
  mitk::StringProperty::Pointer result = mitk::StringProperty::New("testSerialize");
  return result.GetPointer();
}

class mitkPropertyPersistenceInfoTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyPersistenceInfoTestSuite);

  MITK_TEST(CheckDefaultInfo);
  MITK_TEST(SetKey);
  MITK_TEST(SetMimeTypeName);
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

public:

  void setUp() override
  {
    refSerialization = "my_shiny_test_value";
    refProp = mitk::StringProperty::New("my_shiny_test_value");
    info = mitk::PropertyPersistenceInfo::New();
    refSerialization_testFunction = "testSerialize";
    refProp_testFunction = mitk::StringProperty::New("testSerialize");
  }

  void tearDown() override
  {
  }

  void CheckDefaultInfo()
  {
    MITK_TEST_CONDITION_REQUIRED(mitk::PropertyPersistenceInfo::ANY_MIMETYPE_NAME() == mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".any_type", "Testing PropertyPersistenceInfo::ANY_MIMETYPE_NAME()");
    MITK_TEST_CONDITION_REQUIRED(info->GetKey() == "", "Testing PropertyPersistenceInfo::GetKey()");
    MITK_TEST_CONDITION_REQUIRED(info->GetMimeTypeName() == mitk::PropertyPersistenceInfo::ANY_MIMETYPE_NAME(), "Testing PropertyPersistenceInfo::GetMimeTypeName()");

    mitk::BaseProperty::Pointer prop = info->GetDeserializationFunction()(refSerialization);
    MITK_TEST_CONDITION_REQUIRED(dynamic_cast<mitk::StringProperty*>(prop.GetPointer()), "Testing PropertyPersistenceInfo::GetDeserializationFunction() producing a StringProperty");
    MITK_TEST_CONDITION_REQUIRED(prop->GetValueAsString() == refSerialization, "Testing PropertyPersistenceInfo::GetDeserializationFunction() producing a StringProperty with correct value");

    std::string value = info->GetSerializationFunction()(refProp);
    MITK_TEST_CONDITION_REQUIRED(value == refSerialization, "Testing PropertyPersistenceInfo::GetSerializationFunction()");
  }

  void SetKey()
  {
    info->SetKey("newKey");
    MITK_TEST_CONDITION_REQUIRED(info->GetKey() == "newKey", "Testing PropertyPersistenceInfo::SetKey()");
  }

  void SetMimeTypeName()
  {
    info->SetMimeTypeName("newMime");
    MITK_TEST_CONDITION_REQUIRED(info->GetMimeTypeName() == "newMime", "Testing PropertyPersistenceInfo::SetMimeTypeName()");
  }

  void SetDeserializationFunction()
  {
    info->SetDeserializationFunction(testDeserializeFunction);
    mitk::BaseProperty::Pointer prop = info->GetDeserializationFunction()(refSerialization);
    MITK_TEST_CONDITION_REQUIRED(prop->GetValueAsString() == refSerialization_testFunction, "Testing PropertyPersistenceInfo::SetDeserializationFunction() producing a StringProperty with correct value");
  }

  void SetSerializationFunction()
  {
    info->SetSerializationFunction(testSerializeFunction);
    std::string value = info->GetSerializationFunction()(refProp);
    MITK_TEST_CONDITION_REQUIRED(value == refSerialization_testFunction, "Testing PropertyPersistenceInfo::SetSerializationFunction()");
  }

  void serializeByGetValueAsString()
  {
    std::string value = mitk::PropertyPersistenceSerialization::serializeByGetValueAsString(refProp);
    MITK_TEST_CONDITION_REQUIRED(value == refSerialization, "Testing serializeByGetValueAsString()");
  }

  void deserializeToStringProperty()
  {
    mitk::BaseProperty::Pointer prop = mitk::PropertyPersistenceDeserialization::deserializeToStringProperty(refSerialization);
    MITK_TEST_CONDITION_REQUIRED(dynamic_cast<mitk::StringProperty*>(prop.GetPointer()), "Testing deserializeToStringProperty() producing a StringProperty");
    MITK_TEST_CONDITION_REQUIRED(prop->GetValueAsString() == refSerialization, "Testing deserializeToStringProperty() producing a StringProperty with correct value");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyPersistenceInfo)
