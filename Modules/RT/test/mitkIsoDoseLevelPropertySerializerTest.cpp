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
#include <mitkIsoDoseLevelSetProperty.h>
#include <mitkIsoDoseLevelVectorProperty.h>

#include <itkObjectFactoryBase.h>
#include <tinyxml2.h>

class mitkIsoDoseLevelPropertySerializerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkIsoDoseLevelPropertySerializerTestSuite);
  MITK_TEST(RoundTripLevelSet);
  MITK_TEST(RoundTripLevelVector);
  MITK_TEST(NullValuesLoadAsEmpty);
  MITK_TEST(CompareByContent);
  CPPUNIT_TEST_SUITE_END();

  static mitk::IsoDoseLevel::Pointer MakeLevel(double dose, float r, float g, float b, bool isoLine, bool colorWash)
  {
    mitk::IsoDoseLevel::ColorType color;
    color.SetRed(r);
    color.SetGreen(g);
    color.SetBlue(b);
    return mitk::IsoDoseLevel::New(dose, color, isoLine, colorWash);
  }

  static mitk::BaseProperty::Pointer RoundTrip(const mitk::BaseProperty *property)
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

    auto result = serializer->Deserialize(element);
    CPPUNIT_ASSERT_MESSAGE("Deserialization produced a property", result.IsNotNull());
    return result;
  }

  static void AssertEqualLevels(const mitk::IsoDoseLevel &expected, const mitk::IsoDoseLevel &actual)
  {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected.GetDoseValue(), actual.GetDoseValue(), 1e-12);
    CPPUNIT_ASSERT_EQUAL(expected.GetColor(), actual.GetColor());
    CPPUNIT_ASSERT_EQUAL(expected.GetVisibleIsoLine(), actual.GetVisibleIsoLine());
    CPPUNIT_ASSERT_EQUAL(expected.GetVisibleColorWash(), actual.GetVisibleColorWash());
  }

public:
  void RoundTripLevelSet()
  {
    auto levelSet = mitk::IsoDoseLevelSet::New();
    levelSet->SetIsoDoseLevel(MakeLevel(0.5, 0.0f, 0.0f, 1.0f, true, false));
    levelSet->SetIsoDoseLevel(MakeLevel(0.95, 1.0f, 0.5f, 0.25f, false, true));
    levelSet->SetIsoDoseLevel(MakeLevel(1.07, 1.0f, 0.0f, 0.0f, true, true));

    auto property = mitk::IsoDoseLevelSetProperty::New(levelSet);
    const auto resultProperty = RoundTrip(property);
    const auto *result = dynamic_cast<const mitk::IsoDoseLevelSetProperty *>(resultProperty.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Deserialized property has the original type", nullptr != result);

    const auto *resultSet = result->GetValue();
    CPPUNIT_ASSERT_MESSAGE("Deserialized property carries a level set", nullptr != resultSet);
    CPPUNIT_ASSERT_EQUAL(levelSet->Size(), resultSet->Size());

    for (std::size_t i = 0; i < levelSet->Size(); ++i)
      AssertEqualLevels(levelSet->GetIsoDoseLevel(i), resultSet->GetIsoDoseLevel(i));

    CPPUNIT_ASSERT_MESSAGE("Deserialized property compares equal to the original", *property == *result);
  }

  void RoundTripLevelVector()
  {
    auto levelVector = mitk::IsoDoseLevelVector::New();
    levelVector->push_back(MakeLevel(1.2, 0.1f, 0.2f, 0.3f, false, false));
    levelVector->push_back(MakeLevel(0.3, 0.9f, 0.8f, 0.7f, true, false));

    auto property = mitk::IsoDoseLevelVectorProperty::New(levelVector);
    const auto resultProperty = RoundTrip(property);
    const auto *result = dynamic_cast<const mitk::IsoDoseLevelVectorProperty *>(resultProperty.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Deserialized property has the original type", nullptr != result);

    const auto *resultVector = result->GetValue();
    CPPUNIT_ASSERT_MESSAGE("Deserialized property carries a level vector", nullptr != resultVector);
    CPPUNIT_ASSERT_EQUAL(levelVector->size(), resultVector->size());

    for (std::size_t i = 0; i < levelVector->size(); ++i)
      AssertEqualLevels(*levelVector->at(i), *resultVector->at(i));

    CPPUNIT_ASSERT_MESSAGE("Deserialized property compares equal to the original", *property == *result);
  }

  void CompareByContent()
  {
    auto first = mitk::IsoDoseLevelSet::New();
    first->SetIsoDoseLevel(MakeLevel(0.5, 0.0f, 0.0f, 1.0f, true, false));
    auto second = first->Clone();

    CPPUNIT_ASSERT_MESSAGE("Distinct level sets with equal content compare equal",
      *mitk::IsoDoseLevelSetProperty::New(first) == *mitk::IsoDoseLevelSetProperty::New(second));

    second->SetIsoDoseLevel(MakeLevel(0.5, 1.0f, 0.0f, 0.0f, true, false));
    CPPUNIT_ASSERT_MESSAGE("Level sets differing in a level compare unequal",
      !(*mitk::IsoDoseLevelSetProperty::New(first) == *mitk::IsoDoseLevelSetProperty::New(second)));

    CPPUNIT_ASSERT_MESSAGE("Null and non-null level sets compare unequal",
      !(*mitk::IsoDoseLevelSetProperty::New() == *mitk::IsoDoseLevelSetProperty::New(first)));

    CPPUNIT_ASSERT_MESSAGE("Two null level sets compare equal",
      *mitk::IsoDoseLevelSetProperty::New() == *mitk::IsoDoseLevelSetProperty::New());

    auto firstVector = mitk::IsoDoseLevelVector::New();
    firstVector->push_back(MakeLevel(1.2, 0.1f, 0.2f, 0.3f, false, false));
    auto secondVector = mitk::IsoDoseLevelVector::New();
    secondVector->push_back(MakeLevel(1.2, 0.1f, 0.2f, 0.3f, false, false));

    CPPUNIT_ASSERT_MESSAGE("Distinct level vectors with equal content compare equal",
      *mitk::IsoDoseLevelVectorProperty::New(firstVector) == *mitk::IsoDoseLevelVectorProperty::New(secondVector));

    secondVector->push_back(MakeLevel(0.3, 0.9f, 0.8f, 0.7f, true, false));
    CPPUNIT_ASSERT_MESSAGE("Level vectors of different length compare unequal",
      !(*mitk::IsoDoseLevelVectorProperty::New(firstVector) == *mitk::IsoDoseLevelVectorProperty::New(secondVector)));

    auto thirdVector = mitk::IsoDoseLevelVector::New();
    thirdVector->push_back(MakeLevel(1.2, 0.1f, 0.2f, 0.3f, true, false));
    CPPUNIT_ASSERT_MESSAGE("Level vectors differing in a level compare unequal",
      !(*mitk::IsoDoseLevelVectorProperty::New(firstVector) == *mitk::IsoDoseLevelVectorProperty::New(thirdVector)));

    auto nullEntryVector = mitk::IsoDoseLevelVector::New();
    nullEntryVector->push_back(mitk::IsoDoseLevel::Pointer());
    CPPUNIT_ASSERT_MESSAGE("A null entry and a level compare unequal",
      !(*mitk::IsoDoseLevelVectorProperty::New(firstVector) == *mitk::IsoDoseLevelVectorProperty::New(nullEntryVector)));

    auto otherNullEntryVector = mitk::IsoDoseLevelVector::New();
    otherNullEntryVector->push_back(mitk::IsoDoseLevel::Pointer());
    CPPUNIT_ASSERT_MESSAGE("Two null entries compare equal",
      *mitk::IsoDoseLevelVectorProperty::New(nullEntryVector) == *mitk::IsoDoseLevelVectorProperty::New(otherNullEntryVector));

    CPPUNIT_ASSERT_MESSAGE("Null and non-null level vectors compare unequal",
      !(*mitk::IsoDoseLevelVectorProperty::New() == *mitk::IsoDoseLevelVectorProperty::New(firstVector)));

    CPPUNIT_ASSERT_MESSAGE("Two null level vectors compare equal",
      *mitk::IsoDoseLevelVectorProperty::New() == *mitk::IsoDoseLevelVectorProperty::New());
  }

  void NullValuesLoadAsEmpty()
  {
    const auto setProperty = RoundTrip(mitk::IsoDoseLevelSetProperty::New());
    const auto *setResult = dynamic_cast<const mitk::IsoDoseLevelSetProperty *>(setProperty.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Null level set property round-trips", nullptr != setResult);
    CPPUNIT_ASSERT_MESSAGE("Null level set property reloads as an empty level set", nullptr != setResult->GetValue());
    CPPUNIT_ASSERT_EQUAL(mitk::IsoDoseLevelSet::IsoLevelIndexType(0), setResult->GetValue()->Size());

    const auto vectorProperty = RoundTrip(mitk::IsoDoseLevelVectorProperty::New());
    const auto *vectorResult = dynamic_cast<const mitk::IsoDoseLevelVectorProperty *>(vectorProperty.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Null level vector property round-trips", nullptr != vectorResult);
    CPPUNIT_ASSERT_MESSAGE("Null level vector property reloads as an empty level vector", nullptr != vectorResult->GetValue());
    CPPUNIT_ASSERT(vectorResult->GetValue()->empty());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkIsoDoseLevelPropertySerializer)
