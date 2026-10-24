/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkBasePropertySerializer.h>
#include <mitkPlanarFigureControlPointStyleProperty.h>
#include <mitkPlanarFigureVisibilityProperty.h>

#include <itkObjectFactoryBase.h>
#include <tinyxml2.h>

class mitkPlanarFigurePropertySerializerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigurePropertySerializerTestSuite);
  MITK_TEST(RoundTripControlPointStyle);
  MITK_TEST(RoundTripVisibility);
  CPPUNIT_TEST_SUITE_END();

  template <class TProperty>
  static void RoundTrip(TProperty *property)
  {
    const std::string serializerName = std::string(property->GetNameOfClass()) + "Serializer";
    auto serializers = itk::ObjectFactoryBase::CreateAllInstance(serializerName.c_str());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Exactly one serializer registered for " + serializerName, std::size_t(1), serializers.size());

    auto *serializer = dynamic_cast<mitk::BasePropertySerializer *>(serializers.front().GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Registered class is a property serializer", nullptr != serializer);

    tinyxml2::XMLDocument document;
    serializer->SetProperty(property);
    auto *element = serializer->Serialize(document);
    CPPUNIT_ASSERT_MESSAGE("Serialization produced an element", nullptr != element);

    const auto resultProperty = serializer->Deserialize(element);
    const auto *result = dynamic_cast<const TProperty *>(resultProperty.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Deserialization produced a property of the original type", nullptr != result);
    CPPUNIT_ASSERT_EQUAL(property->GetValueAsId(), result->GetValueAsId());
    CPPUNIT_ASSERT_EQUAL(property->GetValueAsString(), result->GetValueAsString());
  }

public:
  void RoundTripControlPointStyle()
  {
    const mitk::EnumerationProperty::IdType id = mitk::PlanarFigureControlPointStyleProperty::Circle;
    auto property = mitk::PlanarFigureControlPointStyleProperty::New(id);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Test id is a valid enumeration value", id, property->GetValueAsId());

    RoundTrip(property.GetPointer());
  }

  void RoundTripVisibility()
  {
    // Not the default value, so that a deserializer that ignores the value fails.
    auto property = mitk::PlanarFigureVisibilityProperty::New(mitk::PlanarFigureVisibilityProperty::Visibility::WhenSelected);
    CPPUNIT_ASSERT_EQUAL(std::string("When selected"), property->GetValueAsString());

    RoundTrip(property.GetPointer());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigurePropertySerializer)
