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
  MITK_TEST(TestSetLayer);
  MITK_TEST(TestSetProperty);
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
    std::string initialName("noName!");
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
    mitk::Point3D indexToBeCompared;
    indexToBeCompared.Fill(0);
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong center of mass index",
                           mitk::Equal(currentIndex, indexToBeCompared));

    indexToBeCompared.SetElement(0, 234.3f);
    indexToBeCompared.SetElement(1, -53);
    indexToBeCompared.SetElement(2, 120);
    label->SetCenterOfMassIndex(indexToBeCompared);
    currentIndex = label->GetCenterOfMassIndex();
    CPPUNIT_ASSERT_MESSAGE("Label has wrong center of mass index", mitk::Equal(currentIndex, indexToBeCompared));
  }

  void TestSetCenterOfMassCoordinates()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    mitk::Point3D currentPoint = label->GetCenterOfMassCoordinates();
    mitk::Point3D pointToBeCompared;
    pointToBeCompared.Fill(0);
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong center of mass index",
                           mitk::Equal(currentPoint, pointToBeCompared));

    pointToBeCompared.SetElement(0, 234.3f);
    pointToBeCompared.SetElement(1, -53);
    pointToBeCompared.SetElement(2, 120);
    label->SetCenterOfMassCoordinates(pointToBeCompared);
    currentPoint = label->GetCenterOfMassCoordinates();
    CPPUNIT_ASSERT_MESSAGE("Label has wrong center of mass index", mitk::Equal(currentPoint, pointToBeCompared));
  }

  void TestSetColor()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    mitk::Color currentColor = label->GetColor();
    mitk::Color colorToBeCompared;
    colorToBeCompared.Set(0, 0, 0);
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

  void TestSetLayer()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    int initialLayer(0);
    int valueToBeCompared = label->GetValue();
    CPPUNIT_ASSERT_MESSAGE("Initial label has wrong layer", initialLayer == valueToBeCompared);

    label->SetLayer(2);
    valueToBeCompared = 2;
    initialLayer = label->GetLayer();
    CPPUNIT_ASSERT_MESSAGE("Label has wrong layer", initialLayer == valueToBeCompared);
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
};

MITK_TEST_SUITE_REGISTRATION(mitkLabel)
