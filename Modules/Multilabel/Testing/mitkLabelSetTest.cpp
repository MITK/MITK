/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLabelSet.h>
#include <mitkLabelSetImage.h>
#include <mitkStringProperty.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkLabelSetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetTestSuite);
  MITK_TEST(TestSetLayer);
  MITK_TEST(TestSetActiveLabel);
  MITK_TEST(TestRemoveLabel);
  MITK_TEST(TestAddLabel);
  MITK_TEST(TestRenameLabel);
  MITK_TEST(TestSetAllLabelsVisible);
  MITK_TEST(TestSetAllLabelsLocked);
  MITK_TEST(TestRemoveAllLabels);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::LabelSet::Pointer m_LabelSet;
  mitk::LabelSet::PixelType m_InitialNumberOfLabels;

  void AddLabels(mitk::LabelSet::PixelType numOfLabels)
  {
    mitk::Label::Pointer label;
    const std::string namePrefix = "Label_";
    const mitk::Color gray(0.5f);

    for (mitk::Label::PixelType i = 0; i < numOfLabels; ++i)
    {
      label = mitk::Label::New();
      label->SetName(namePrefix + std::to_string(i));
      label->SetValue(i);
      label->SetVisible((i % 2 == 0));
      label->SetLayer(i % 3);
      label->SetColor(gray);

      m_LabelSet->AddLabel(label);
    }
  }

public:
  void setUp() override
  {
    m_InitialNumberOfLabels = 6;
    m_LabelSet = mitk::LabelSet::New();

    this->AddLabels(m_InitialNumberOfLabels);
    m_LabelSet->SetLayer(0);
    m_LabelSet->SetActiveLabel(0);
  }

  void tearDown() override
  {
    m_LabelSet = nullptr;
  }

  void TestSetLayer()
  {
    CPPUNIT_ASSERT_MESSAGE("Wrong initial layer", m_LabelSet->GetLayer() == 0);

    m_LabelSet->SetLayer(1);
    CPPUNIT_ASSERT_MESSAGE("Wrong layer", m_LabelSet->GetLayer() == 1);
  }

  void TestSetActiveLabel()
  {
    CPPUNIT_ASSERT_MESSAGE("Wrong initial active label", m_LabelSet->GetActiveLabel()->GetValue() == 0);

    m_LabelSet->SetActiveLabel(1);
    CPPUNIT_ASSERT_MESSAGE("Wrong layer", m_LabelSet->GetActiveLabel()->GetValue() == 1);
  }

  void TestRemoveLabel()
  {
    CPPUNIT_ASSERT_MESSAGE("Wrong initial number of label", m_LabelSet->GetNumberOfLabels() == m_InitialNumberOfLabels);

    // Remove a label that is not the active label
    m_LabelSet->SetActiveLabel(2);
    m_LabelSet->RemoveLabel(1);

    mitk::LabelSet::PixelType numLabels = m_InitialNumberOfLabels - 1;

    CPPUNIT_ASSERT_MESSAGE("Label was not removed", m_LabelSet->ExistLabel(1) == false);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of label", m_LabelSet->GetNumberOfLabels() == numLabels);
    CPPUNIT_ASSERT_MESSAGE("Wrong active label", m_LabelSet->GetActiveLabel()->GetValue() == 2);

    // Remove active label - now the succeeding label should be active
    m_LabelSet->RemoveLabel(2);
    CPPUNIT_ASSERT_MESSAGE("Wrong layer", m_LabelSet->GetActiveLabel()->GetValue() == 3);
    CPPUNIT_ASSERT_MESSAGE("Label was not removed", m_LabelSet->ExistLabel(2) == false);
    CPPUNIT_ASSERT_MESSAGE("Wrong initial number of label", m_LabelSet->GetNumberOfLabels() == --numLabels);
  }

  void TestAddLabel()
  {
    auto newLabel = mitk::Label::New();
    newLabel->SetValue(1);
    m_LabelSet->AddLabel(newLabel);

    // Since label with value 1 already exists the new label will get the value m_InitialNumberOfValues
    CPPUNIT_ASSERT_MESSAGE("Wrong label value", m_LabelSet->GetActiveLabel()->GetValue() == m_InitialNumberOfLabels);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of label", m_LabelSet->GetNumberOfLabels() == static_cast<decltype(m_LabelSet->GetNumberOfLabels())>(m_InitialNumberOfLabels + 1));
  }

  void TestRenameLabel()
  {
    const mitk::Color white(1.0f);
    const std::string name = "MyAwesomeLabel";

    m_LabelSet->RenameLabel(0, name, white);

    const auto* label = m_LabelSet->GetLabel(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong label name", label->GetName() == name );

    const auto& color = label->GetColor();
    CPPUNIT_ASSERT_MESSAGE("Wrong color", color == white);
  }

  void TestSetAllLabelsVisible()
  {
    const auto numLabels = static_cast<mitk::LabelSet::PixelType>(m_LabelSet->GetNumberOfLabels());

    m_LabelSet->SetAllLabelsVisible(true);

    for (mitk::LabelSet::PixelType i = 0; i < numLabels; ++i)
      CPPUNIT_ASSERT_MESSAGE("Label not visible", m_LabelSet->GetLabel(i)->GetVisible() == true);

    m_LabelSet->SetAllLabelsVisible(false);

    for (mitk::LabelSet::PixelType i = 0; i < numLabels; ++i)
      CPPUNIT_ASSERT_MESSAGE("Label visible", m_LabelSet->GetLabel(i)->GetVisible() == false);
  }

  void TestSetAllLabelsLocked()
  {
    const auto numLabels = static_cast<mitk::LabelSet::PixelType>(m_LabelSet->GetNumberOfLabels());

    m_LabelSet->SetAllLabelsLocked(true);

    for (mitk::LabelSet::PixelType i = 0; i < numLabels; ++i)
      CPPUNIT_ASSERT_MESSAGE("Label not locked", m_LabelSet->GetLabel(i)->GetLocked() == true);

    m_LabelSet->SetAllLabelsLocked(false);

    for (mitk::LabelSet::PixelType i = 0; i < numLabels; ++i)
      CPPUNIT_ASSERT_MESSAGE("Label locked", m_LabelSet->GetLabel(i)->GetLocked() == false);
  }

  void TestRemoveAllLabels()
  {
    m_LabelSet->RemoveAllLabels();
    CPPUNIT_ASSERT_MESSAGE("Not all labels were removed", m_LabelSet->GetNumberOfLabels() == 0);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSet)
