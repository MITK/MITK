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
    for (mitk::Label::PixelType i = 0; i < numOfLabels; ++i)
    {
      mitk::Label::Pointer label = mitk::Label::New();
      std::stringstream nameStream;
      nameStream << "Label_";
      nameStream << i;
      label->SetName(nameStream.str());
      label->SetValue(i);
      label->SetVisible( (i%20 == 0) );
      label->SetLayer( i%5);
      mitk::Color color;
      color.Set(0.3f, 0.25f, 1.0f);
      label->SetColor(color);

      m_LabelSet->AddLabel(label);
    }
  }

public:

  void setUp() override
  {
     m_InitialNumberOfLabels = 200;
     m_LabelSet = mitk::LabelSet::New();

     this->AddLabels(m_InitialNumberOfLabels);
     m_LabelSet->SetLayer(0);
     m_LabelSet->SetActiveLabel(0);
  }

  void tearDown() override
  {
    // Reset label set
    m_LabelSet = nullptr;
  }

  void TestSetLayer()
  {
    CPPUNIT_ASSERT_MESSAGE("Wrong initial layer", m_LabelSet->GetLayer() == 0);
    m_LabelSet->SetLayer(3);
    CPPUNIT_ASSERT_MESSAGE("Wrong layer", m_LabelSet->GetLayer() == 3);
  }

  void TestSetActiveLabel()
  {
    CPPUNIT_ASSERT_MESSAGE("Wrong initial active label", m_LabelSet->GetActiveLabel()->GetValue() == 0);
    m_LabelSet->SetActiveLabel(145);
    CPPUNIT_ASSERT_MESSAGE("Wrong layer", m_LabelSet->GetActiveLabel()->GetValue() == 145);
  }

  void TestRemoveLabel()
  {
    CPPUNIT_ASSERT_MESSAGE("Wrong initial number of label", static_cast<unsigned short>(m_LabelSet->GetNumberOfLabels()) == m_InitialNumberOfLabels);

    // Remove a label that is not the active label
    m_LabelSet->SetActiveLabel(12);
    m_LabelSet->RemoveLabel(56);
    unsigned int numLabels = m_InitialNumberOfLabels-1;
    CPPUNIT_ASSERT_MESSAGE("Label was not removed", m_LabelSet->ExistLabel(56) == false);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of label", m_LabelSet->GetNumberOfLabels() == numLabels);
    CPPUNIT_ASSERT_MESSAGE("Wrong active label", m_LabelSet->GetActiveLabel()->GetValue() == 12);

    // Remove active label - now the succeeding label should be active
    m_LabelSet->RemoveLabel(12);
    CPPUNIT_ASSERT_MESSAGE("Wrong layer", m_LabelSet->GetActiveLabel()->GetValue() == 13);
    CPPUNIT_ASSERT_MESSAGE("Label was not removed", m_LabelSet->ExistLabel(12) == false);
    numLabels = m_InitialNumberOfLabels-2;
    CPPUNIT_ASSERT_MESSAGE("Wrong initial number of label", m_LabelSet->GetNumberOfLabels() == numLabels);
  }

  void TestAddLabel()
  {
    CPPUNIT_ASSERT_MESSAGE("Wrong initial number of label", m_LabelSet->GetNumberOfLabels() == m_InitialNumberOfLabels);
    mitk::Label::Pointer newLabel = mitk::Label::New();
    newLabel->SetValue(199);
    m_LabelSet->AddLabel(newLabel);
    // Since label with value 199 already exists the new label will get the value 200
    CPPUNIT_ASSERT_MESSAGE("Wrong label value", m_LabelSet->GetActiveLabel()->GetValue() == 200);
    unsigned int numLabels = m_InitialNumberOfLabels+1;
    CPPUNIT_ASSERT_MESSAGE("Wrong number of label", m_LabelSet->GetNumberOfLabels() == numLabels);

    // Add new labels until the maximum number of labels is reached.
    // Adding more labels should have no effect.
    this->AddLabels(mitk::Label::MAX_LABEL_VALUE);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of label", m_LabelSet->GetNumberOfLabels() == 65536);
    mitk::Label* activeLabel = m_LabelSet->GetActiveLabel();
    CPPUNIT_ASSERT_MESSAGE("Wrong value of active label", activeLabel->GetValue() == mitk::Label::MAX_LABEL_VALUE);
  }

  void TestRenameLabel()
  {
    mitk::Color color;
    color.Set(1.0f,1.0f,1.0f);
    std::string name("MyAwesomeLabel");
    m_LabelSet->RenameLabel(0, name, color);
    mitk::Label* label = m_LabelSet->GetLabel(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong label name", label->GetName().compare("MyAwesomeLabel") == 0);
    mitk::Color color2 = label->GetColor();
    CPPUNIT_ASSERT_MESSAGE("Wrong color", (color2.GetBlue() == 1.0f && color2.GetGreen() == 1.0f && color2.GetRed() == 1.0f));
  }

  void TestSetAllLabelsVisible()
  {
    m_LabelSet->SetAllLabelsVisible(true);
    for(mitk::LabelSet::PixelType i=0; i < m_LabelSet->GetNumberOfLabels(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("Label not visible", m_LabelSet->GetLabel(i)->GetVisible() == true);
    }

    m_LabelSet->SetAllLabelsVisible(false);
    for(mitk::LabelSet::PixelType i=0; i < m_LabelSet->GetNumberOfLabels(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("Label visible", m_LabelSet->GetLabel(i)->GetVisible() == false);
    }
  }

  void TestSetAllLabelsLocked()
  {
    m_LabelSet->SetAllLabelsLocked(true);
    for(mitk::LabelSet::PixelType i=0; i < m_LabelSet->GetNumberOfLabels(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("Label not locked", m_LabelSet->GetLabel(i)->GetLocked() == true);
    }

    m_LabelSet->SetAllLabelsLocked(false);
    for(mitk::LabelSet::PixelType i=0; i < m_LabelSet->GetNumberOfLabels(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("Label locked", m_LabelSet->GetLabel(i)->GetLocked() == false);
    }
  }

  void TestRemoveAllLabels()
  {
    m_LabelSet->RemoveAllLabels();
    CPPUNIT_ASSERT_MESSAGE("Not all labels were removed", m_LabelSet->GetNumberOfLabels() == 0);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSet)
