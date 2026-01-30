/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <mitkColorProperty.h>
#include <mitkProperties.h>
#include <mitkPropertyList.h>
#include <mitkStringProperty.h>

class mitkPropertyJsonSerializationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyJsonSerializationTestSuite);
  MITK_TEST(TestStringPropertySerialization);
  MITK_TEST(TestIntPropertySerialization);
  MITK_TEST(TestFloatPropertySerialization);
  MITK_TEST(TestDoublePropertySerialization);
  MITK_TEST(TestBoolPropertySerialization);
  MITK_TEST(TestColorPropertySerialization);
  MITK_TEST(TestPropertyListSerialization);
  MITK_TEST(TestPropertyListDeserialization);
  MITK_TEST(TestNullPropertyThrows);
  MITK_TEST(TestNullPropertyListThrows);
  MITK_TEST(TestUnsupportedJsonTypeThrows);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
  }

  void tearDown() override
  {
  }

  void TestStringPropertySerialization()
  {
    auto prop = mitk::StringProperty::New("test value");

    // Serialize
    auto json = mitk::ConvertPropertyToSelfContainedJson(prop);

    // String properties should serialize directly as JSON string
    CPPUNIT_ASSERT_MESSAGE("StringProperty should serialize as JSON string",
                           json.is_string());
    CPPUNIT_ASSERT_EQUAL(std::string("test value"), json.get<std::string>());

    // Deserialize
    auto restored = mitk::ConvertPropertyFromSelfContainedJson(json);
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should not be null", restored.IsNotNull());

    auto restoredString = dynamic_cast<mitk::StringProperty*>(restored.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should be StringProperty", restoredString != nullptr);
    CPPUNIT_ASSERT_EQUAL(std::string("test value"), std::string(restoredString->GetValue()));
  }

  void TestIntPropertySerialization()
  {
    auto prop = mitk::IntProperty::New(42);

    // Serialize
    auto json = mitk::ConvertPropertyToSelfContainedJson(prop);

    // Int properties should serialize directly as JSON integer
    CPPUNIT_ASSERT_MESSAGE("IntProperty should serialize as JSON integer",
                           json.is_number_integer());
    CPPUNIT_ASSERT_EQUAL(42, json.get<int>());

    // Deserialize
    auto restored = mitk::ConvertPropertyFromSelfContainedJson(json);
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should not be null", restored.IsNotNull());

    auto restoredInt = dynamic_cast<mitk::IntProperty*>(restored.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should be IntProperty", restoredInt != nullptr);
    CPPUNIT_ASSERT_EQUAL(42, restoredInt->GetValue());
  }

  void TestFloatPropertySerialization()
  {
    auto prop = mitk::FloatProperty::New(3.14f);

    // Serialize
    auto json = mitk::ConvertPropertyToSelfContainedJson(prop);

    // Float properties should serialize directly as JSON number
    CPPUNIT_ASSERT_MESSAGE("FloatProperty should serialize as JSON number",
                           json.is_number());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.14f, json.get<float>(), 0.001f);

    // Deserialize
    auto restored = mitk::ConvertPropertyFromSelfContainedJson(json);
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should not be null", restored.IsNotNull());

    auto restoredFloat = dynamic_cast<mitk::FloatProperty*>(restored.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should be FloatProperty", restoredFloat != nullptr);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.14f, restoredFloat->GetValue(), 0.001f);
  }

  void TestDoublePropertySerialization()
  {
    auto prop = mitk::DoubleProperty::New(3.14159265359);

    // Serialize
    auto json = mitk::ConvertPropertyToSelfContainedJson(prop);

    // DoubleProperty should serialize as complex type to avoid conversion inconsistency
    CPPUNIT_ASSERT_MESSAGE("DoubleProperty should serialize as JSON object",
                           json.is_object());
    CPPUNIT_ASSERT_MESSAGE("DoubleProperty JSON should have 'type' field",
                           json.contains("type"));
    CPPUNIT_ASSERT_MESSAGE("DoubleProperty JSON should have 'value' field",
                           json.contains("value"));
    CPPUNIT_ASSERT_EQUAL(std::string("DoubleProperty"), json["type"].get<std::string>());

    // Deserialize
    auto restored = mitk::ConvertPropertyFromSelfContainedJson(json);
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should not be null", restored.IsNotNull());

    // Should deserialize back to DoubleProperty, not FloatProperty
    auto restoredDouble = dynamic_cast<mitk::DoubleProperty*>(restored.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should be DoubleProperty", restoredDouble != nullptr);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.14159265359, restoredDouble->GetValue(), 0.0000001);
  }

  void TestBoolPropertySerialization()
  {
    auto propTrue = mitk::BoolProperty::New(true);
    auto propFalse = mitk::BoolProperty::New(false);

    // Serialize
    auto jsonTrue = mitk::ConvertPropertyToSelfContainedJson(propTrue);
    auto jsonFalse = mitk::ConvertPropertyToSelfContainedJson(propFalse);

    // Bool properties should serialize directly as JSON boolean
    CPPUNIT_ASSERT_MESSAGE("BoolProperty(true) should serialize as JSON boolean",
                           jsonTrue.is_boolean());
    CPPUNIT_ASSERT_MESSAGE("BoolProperty(false) should serialize as JSON boolean",
                           jsonFalse.is_boolean());
    CPPUNIT_ASSERT_EQUAL(true, jsonTrue.get<bool>());
    CPPUNIT_ASSERT_EQUAL(false, jsonFalse.get<bool>());

    // Deserialize
    auto restoredTrue = mitk::ConvertPropertyFromSelfContainedJson(jsonTrue);
    auto restoredFalse = mitk::ConvertPropertyFromSelfContainedJson(jsonFalse);

    CPPUNIT_ASSERT_MESSAGE("Deserialized true property should not be null", restoredTrue.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Deserialized false property should not be null", restoredFalse.IsNotNull());

    auto restoredBoolTrue = dynamic_cast<mitk::BoolProperty*>(restoredTrue.GetPointer());
    auto restoredBoolFalse = dynamic_cast<mitk::BoolProperty*>(restoredFalse.GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Deserialized true property should be BoolProperty", restoredBoolTrue != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Deserialized false property should be BoolProperty", restoredBoolFalse != nullptr);
    CPPUNIT_ASSERT_EQUAL(true, restoredBoolTrue->GetValue());
    CPPUNIT_ASSERT_EQUAL(false, restoredBoolFalse->GetValue());
  }

  void TestColorPropertySerialization()
  {
    mitk::Color color;
    color.SetRed(0.5f);
    color.SetGreen(0.7f);
    color.SetBlue(0.3f);
    auto prop = mitk::ColorProperty::New(color);

    // Serialize
    auto json = mitk::ConvertPropertyToSelfContainedJson(prop);

    // ColorProperty is complex, should have type and value
    CPPUNIT_ASSERT_MESSAGE("ColorProperty should serialize as JSON object",
                           json.is_object());
    CPPUNIT_ASSERT_MESSAGE("ColorProperty JSON should have 'type' field",
                           json.contains("type"));
    CPPUNIT_ASSERT_MESSAGE("ColorProperty JSON should have 'value' field",
                           json.contains("value"));
    CPPUNIT_ASSERT_EQUAL(std::string("ColorProperty"), json["type"].get<std::string>());

    // Deserialize
    auto restored = mitk::ConvertPropertyFromSelfContainedJson(json);
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should not be null", restored.IsNotNull());

    auto restoredColor = dynamic_cast<mitk::ColorProperty*>(restored.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Deserialized property should be ColorProperty", restoredColor != nullptr);

    auto restoredValue = restoredColor->GetColor();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5f, restoredValue.GetRed(), 0.001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.7f, restoredValue.GetGreen(), 0.001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3f, restoredValue.GetBlue(), 0.001f);
  }

  void TestPropertyListSerialization()
  {
    auto propList = mitk::PropertyList::New();
    propList->SetStringProperty("name", "Test Node");
    propList->SetIntProperty("count", 5);
    propList->SetFloatProperty("opacity", 0.8f);
    propList->SetBoolProperty("visible", true);

    // Serialize
    auto json = mitk::ConvertPropertyListToSelfContainedJson(propList);

    // Should be an object with all properties
    CPPUNIT_ASSERT_MESSAGE("PropertyList should serialize as JSON object",
                           json.is_object());
    CPPUNIT_ASSERT_MESSAGE("JSON should contain 'name' key", json.contains("name"));
    CPPUNIT_ASSERT_MESSAGE("JSON should contain 'count' key", json.contains("count"));
    CPPUNIT_ASSERT_MESSAGE("JSON should contain 'opacity' key", json.contains("opacity"));
    CPPUNIT_ASSERT_MESSAGE("JSON should contain 'visible' key", json.contains("visible"));

    CPPUNIT_ASSERT_EQUAL(std::string("Test Node"), json["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(5, json["count"].get<int>());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8f, json["opacity"].get<float>(), 0.001f);
    CPPUNIT_ASSERT_EQUAL(true, json["visible"].get<bool>());
  }

  void TestPropertyListDeserialization()
  {
    nlohmann::json json = {
      {"name", "Restored Node"},
      {"count", 10},
      {"opacity", 0.5f},
      {"visible", false}
    };

    auto propList = mitk::ConvertPropertyListFromSelfContainedJson(json);
    CPPUNIT_ASSERT_MESSAGE("PropertyList should not be null", propList.IsNotNull());

    std::string name;
    int count;
    float opacity;
    bool visible;

    CPPUNIT_ASSERT_MESSAGE("Should get 'name' property", propList->GetStringProperty("name", name));
    CPPUNIT_ASSERT_MESSAGE("Should get 'count' property", propList->GetIntProperty("count", count));
    CPPUNIT_ASSERT_MESSAGE("Should get 'opacity' property", propList->GetFloatProperty("opacity", opacity));
    CPPUNIT_ASSERT_MESSAGE("Should get 'visible' property", propList->GetBoolProperty("visible", visible));

    CPPUNIT_ASSERT_EQUAL(std::string("Restored Node"), name);
    CPPUNIT_ASSERT_EQUAL(10, count);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5f, opacity, 0.001f);
    CPPUNIT_ASSERT_EQUAL(false, visible);
  }

  void TestNullPropertyThrows()
  {
    CPPUNIT_ASSERT_THROW(mitk::ConvertPropertyToSelfContainedJson(nullptr), mitk::Exception);
  }

  void TestNullPropertyListThrows()
  {
    CPPUNIT_ASSERT_THROW(mitk::ConvertPropertyListToSelfContainedJson(nullptr), mitk::Exception);
  }

  void TestUnsupportedJsonTypeThrows()
  {
    // JSON array is not a supported type for property deserialization
    nlohmann::json jsonArray = nlohmann::json::array({1, 2, 3});
    CPPUNIT_ASSERT_THROW(mitk::ConvertPropertyFromSelfContainedJson(jsonArray), mitk::Exception);

    // JSON null is not a supported type
    nlohmann::json jsonNull = nullptr;
    CPPUNIT_ASSERT_THROW(mitk::ConvertPropertyFromSelfContainedJson(jsonNull), mitk::Exception);

    // Non-object JSON for PropertyListFromJson should throw
    nlohmann::json jsonString = "not an object";
    CPPUNIT_ASSERT_THROW(mitk::ConvertPropertyListFromSelfContainedJson(jsonString), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyJsonSerialization)
