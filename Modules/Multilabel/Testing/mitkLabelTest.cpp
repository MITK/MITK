/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLabel.h"
#include "mitkStringProperty.h"
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkLabelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelTestSuite);
  MITK_TEST(TestSetLock);
  MITK_TEST(TestSetVisibility);
  MITK_TEST(TestSetOpacity);
  MITK_TEST(TestSetName);
  MITK_TEST(TestSetCenterOfMassIndex);
  MITK_TEST(TestSetCenterOfMassCoordinates);
  MITK_TEST(TestSetColor);
  MITK_TEST(TestSetValue);
  MITK_TEST(TestSetProperty);
  MITK_TEST(TestAlgorithmFunctions);
  MITK_TEST(TestDICOMFunctions);
  CPPUNIT_TEST_SUITE_END();

public:
  void TestSetLock()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    CPPUNIT_ASSERT_MESSAGE("Initial label not locked", label->GetLocked() == true);

    label->SetLocked(false);
    CPPUNIT_ASSERT_MESSAGE("Label should not be locked", label->GetLocked() == false);
  }

  void TestSetVisibility()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    CPPUNIT_ASSERT_MESSAGE("Initial label not visible", label->GetVisible() == true);

    label->SetVisible(false);
    CPPUNIT_ASSERT_MESSAGE("Label should not be visible", label->GetVisible() == false);
  }

  void TestSetOpacity()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong opacity", mitk::Equal(label->GetOpacity(), 0.6f));

    label->SetOpacity(0.32f);
    CPPUNIT_ASSERT_MESSAGE("Label has wrong opacity", mitk::Equal(label->GetOpacity(), 0.32f));
  }

  void TestSetName()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    std::string initialName("Unknown label name");
    std::string labelName = label->GetName();
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong name", initialName.compare(labelName) == 0);

    label->SetName("AwesomeLabel");
    labelName = label->GetName();
    CPPUNIT_ASSERT_MESSAGE("Label has wrong name", labelName.compare("AwesomeLabel") == 0);
  }

  void TestSetCenterOfMassIndex()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    mitk::Point3D currentIndex = label->GetCenterOfMassIndex();
    mitk::Point3D other;
    mitk::Point3D indexToBeCompared;
    indexToBeCompared.Fill(0);
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong center of mass index",
                           mitk::Equal(currentIndex, indexToBeCompared));

    indexToBeCompared.SetElement(0, 234.3f);
    indexToBeCompared.SetElement(1, -53);
    indexToBeCompared.SetElement(2, 120);
    label->UpdateCenterOfMass(indexToBeCompared,other);
    currentIndex = label->GetCenterOfMassIndex();
    CPPUNIT_ASSERT_MESSAGE("Label has wrong center of mass index", mitk::Equal(currentIndex, indexToBeCompared));
  }

  void TestSetCenterOfMassCoordinates()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    mitk::Point3D currentPoint = label->GetCenterOfMassCoordinates();
    mitk::Point3D other;
    mitk::Point3D pointToBeCompared;
    pointToBeCompared.Fill(0);
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong center of mass index",
                           mitk::Equal(currentPoint, pointToBeCompared));

    pointToBeCompared.SetElement(0, 234.3f);
    pointToBeCompared.SetElement(1, -53);
    pointToBeCompared.SetElement(2, 120);
    label->UpdateCenterOfMass(other, pointToBeCompared);
    currentPoint = label->GetCenterOfMassCoordinates();
    CPPUNIT_ASSERT_MESSAGE("Label has wrong center of mass index", mitk::Equal(currentPoint, pointToBeCompared));
  }

  void TestSetColor()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    mitk::Color currentColor = label->GetColor();
    mitk::Color colorToBeCompared;
    colorToBeCompared.Set(1., 1., 1.);
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong color", currentColor.GetBlue() == colorToBeCompared.GetBlue());
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong color", currentColor.GetGreen() == colorToBeCompared.GetGreen());
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong color", currentColor.GetRed() == colorToBeCompared.GetRed());

    colorToBeCompared.Set(0.4f, 0.3f, 1.0f);
    label->SetColor(colorToBeCompared);
    currentColor = label->GetColor();
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong color", currentColor.GetBlue() == colorToBeCompared.GetBlue());
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong color", currentColor.GetGreen() == colorToBeCompared.GetGreen());
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong color", currentColor.GetRed() == colorToBeCompared.GetRed());
  }

  void TestSetValue()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    mitk::Label::PixelType initialValue(0);
    mitk::Label::PixelType valueToBeCompared = label->GetValue();
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong value", initialValue == valueToBeCompared);

    label->SetValue(12345);
    valueToBeCompared = 12345;
    initialValue = label->GetValue();
    CPPUNIT_ASSERT_MESSAGE("Label has wrong value", initialValue == valueToBeCompared);
  }

  void TestSetProperty()
  {
    mitk::Label::Pointer label = mitk::Label::New();

    mitk::StringProperty::Pointer prop = mitk::StringProperty::New("abc");
    label->SetProperty("cba", prop);
    std::string propVal;
    label->GetStringProperty("cba", propVal);
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong value", propVal.compare("abc") == 0);
  }

  void TestAlgorithmFunctions()
  {
    mitk::Label::Pointer label = mitk::Label::New();

    // --- Check all AlgorithmType values and their string representations ---
    struct AlgoPair { mitk::Label::AlgorithmType type; std::string str; };
    std::vector<AlgoPair> algoPairs = {
        {mitk::Label::AlgorithmType::Undefined, ""},
        {mitk::Label::AlgorithmType::MANUAL, "MANUAL"},
        {mitk::Label::AlgorithmType::SEMIAUTOMATIC, "SEMIAUTOMATIC"},
        {mitk::Label::AlgorithmType::AUTOMATIC, "AUTOMATIC"}
    };

    for (const auto& pair : algoPairs)
    {
      label->SetAlgorithmType(pair.type);
      CPPUNIT_ASSERT(label->GetAlgorithmType() == pair.type);
      CPPUNIT_ASSERT(label->GetAlgorithmTypeStr() == pair.str);
    }

    // --- Algorithm Name ---
    std::string algoName1 = "ToolA";
    label->SetAlgorithmName(algoName1);
    CPPUNIT_ASSERT(label->GetAlgorithmName() == algoName1);

    // --- AddToolUse: same type, new tool ---
    label->SetAlgorithmType(mitk::Label::AlgorithmType::MANUAL);
    label->AddToolUse(mitk::Label::AlgorithmType::MANUAL, "ToolB");
    CPPUNIT_ASSERT(label->GetAlgorithmType() == mitk::Label::AlgorithmType::MANUAL);
    std::string name = label->GetAlgorithmName();
    CPPUNIT_ASSERT(name == "ToolA|ToolB");

    // --- AddToolUse: different type, new tool ---
    label->AddToolUse(mitk::Label::AlgorithmType::AUTOMATIC, "nnUNet");
    CPPUNIT_ASSERT(label->GetAlgorithmType() == mitk::Label::AlgorithmType::SEMIAUTOMATIC);
    name = label->GetAlgorithmName();
    CPPUNIT_ASSERT(name == "ToolA|ToolB|nnUNet");

    // --- Check that AddToolUse does not duplicate existing tool ---
    label->AddToolUse(mitk::Label::AlgorithmType::AUTOMATIC, "nnUNet");
    CPPUNIT_ASSERT(label->GetAlgorithmType() == mitk::Label::AlgorithmType::SEMIAUTOMATIC);
    name = label->GetAlgorithmName();
    CPPUNIT_ASSERT(name == "ToolA|ToolB|nnUNet");
  }

  void TestDICOMFunctions()
  {
    mitk::Label::Pointer label = mitk::Label::New();

    // --- AnatomicRegion with multiple entries and modifiers ---
    mitk::DICOMCodeSequenceWithModifiers region1("T-12345", "SRT", "Heart");
    region1.AddModifier(mitk::DICOMCodeSequence("T-54321", "SRT", "Anterior"));
    label->SetAnatomicRegion(region1, 0);

    mitk::DICOMCodeSequenceWithModifiers region2("T-67890", "SRT", "Lung");
    label->SetAnatomicRegion(region2, 1);

    CPPUNIT_ASSERT(label->GetAnatomicRegionCount() == 2);

    auto retrievedRegion1 = label->GetAnatomicRegion(0);
    CPPUNIT_ASSERT(retrievedRegion1.GetValue() == "T-12345");
    CPPUNIT_ASSERT(retrievedRegion1.GetModifiers().size() == 1);
    CPPUNIT_ASSERT(retrievedRegion1.GetModifiers()[0].GetValue() == "T-54321");

    auto retrievedRegion2 = label->GetAnatomicRegion(1);
    CPPUNIT_ASSERT(retrievedRegion2.GetValue() == "T-67890");
    CPPUNIT_ASSERT(retrievedRegion2.GetModifiers().empty());

    // --- PrimaryAnatomicStructure with multiple entries ---
    mitk::DICOMCodeSequenceWithModifiers primary1("T-11111", "SRT", "LeftVentricle");
    primary1.AddModifier(mitk::DICOMCodeSequence("T-22222", "SRT", "Base"));
    label->SetPrimaryAnatomicStructure(primary1, 0);

    mitk::DICOMCodeSequenceWithModifiers primary2("T-33333", "SRT", "RightVentricle");
    label->SetPrimaryAnatomicStructure(primary2, 1);

    CPPUNIT_ASSERT(label->GetPrimaryAnatomicStructureCount() == 2);

    auto retrievedPrimary1 = label->GetPrimaryAnatomicStructure(0);
    CPPUNIT_ASSERT(retrievedPrimary1.GetValue() == "T-11111");
    CPPUNIT_ASSERT(retrievedPrimary1.GetModifiers().size() == 1);
    CPPUNIT_ASSERT(retrievedPrimary1.GetModifiers()[0].GetValue() == "T-22222");

    auto retrievedPrimary2 = label->GetPrimaryAnatomicStructure(1);
    CPPUNIT_ASSERT(retrievedPrimary2.GetValue() == "T-33333");
    CPPUNIT_ASSERT(retrievedPrimary2.GetModifiers().empty());

    // --- SegmentedPropertyCategory ---
    mitk::DICOMCodeSequence segCatCode("1234", "SRT", "Organ");
    label->SetSegmentedPropertyCategory(segCatCode);
    auto retrievedSegCat = label->GetSegmentedPropertyCategory();
    CPPUNIT_ASSERT(retrievedSegCat.has_value());
    CPPUNIT_ASSERT(retrievedSegCat->GetValue() == "1234");

    // --- SegmentedPropertyType with modifiers ---
    mitk::DICOMCodeSequenceWithModifiers segTypeCode("5678", "SRT", "Tissue");
    segTypeCode.AddModifier(mitk::DICOMCodeSequence("8765", "SRT", "Modified"));
    label->SetSegmentedPropertyType(segTypeCode);

    auto retrievedSegType = label->GetSegmentedPropertyType();
    CPPUNIT_ASSERT(retrievedSegType.has_value());
    CPPUNIT_ASSERT(retrievedSegType->GetValue() == "5678");
    CPPUNIT_ASSERT(retrievedSegType->GetModifiers().size() == 1);
    CPPUNIT_ASSERT(retrievedSegType->GetModifiers()[0].GetValue() == "8765");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabel)
