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
#include <mitkRegEvalStyleProperty.h>
#include <mitkRegEvalWipeStyleProperty.h>
#include <mitkRegVisColorStyleProperty.h>
#include <mitkRegVisDirectionProperty.h>
#include <mitkRegVisStyleProperty.h>

#include <itkObjectFactoryBase.h>
#include <tinyxml2.h>

class mitkRegPropertySerializerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRegPropertySerializerTestSuite);
  MITK_TEST(RoundTripRegEvalStyle);
  MITK_TEST(RoundTripRegEvalWipeStyle);
  MITK_TEST(RoundTripRegVisColorStyle);
  MITK_TEST(RoundTripRegVisDirection);
  MITK_TEST(RoundTripRegVisStyle);
  CPPUNIT_TEST_SUITE_END();

  template <typename TProperty>
  static void RoundTrip(mitk::EnumerationProperty::IdType id)
  {
    auto property = TProperty::New(id);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Test id is a valid enumeration value", id, property->GetValueAsId());

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
  void RoundTripRegEvalStyle()
  {
    RoundTrip<mitk::RegEvalStyleProperty>(2);
  }

  void RoundTripRegEvalWipeStyle()
  {
    RoundTrip<mitk::RegEvalWipeStyleProperty>(1);
  }

  void RoundTripRegVisColorStyle()
  {
    RoundTrip<mitk::RegVisColorStyleProperty>(1);
  }

  void RoundTripRegVisDirection()
  {
    RoundTrip<mitk::RegVisDirectionProperty>(0);
  }

  void RoundTripRegVisStyle()
  {
    RoundTrip<mitk::RegVisStyleProperty>(2);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRegPropertySerializer)
