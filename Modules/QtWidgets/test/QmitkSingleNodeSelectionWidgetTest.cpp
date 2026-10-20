/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkSingleNodeSelectionWidget.h>

#include <mitkNodePredicateFunction.h>
#include <mitkStandaloneDataStorage.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vector>

namespace
{
  using NodeList = QmitkSingleNodeSelectionWidget::NodeList;
  using Emissions = std::vector<NodeList>;

  /** Renders emissions like "[{image1}, {}]", so failed assertions show what was emitted. */
  std::string Describe(const Emissions& emissions)
  {
    std::string result = "[";

    for (std::size_t i = 0; i < emissions.size(); ++i)
    {
      if (i > 0)
        result += ", ";

      result += "{";

      for (qsizetype j = 0; j < emissions[i].size(); ++j)
      {
        if (j > 0)
          result += ", ";

        result += emissions[i][j]->GetName();
      }

      result += "}";
    }

    return result + "]";
  }

  class TestWidget : public QmitkSingleNodeSelectionWidget
  {
  public:
    TestWidget()
    {
      this->connect(this, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, [this](NodeList selection) {
        m_Emissions.push_back(selection);
      });
    }

    void ClickClearButton()
    {
      this->OnClearSelection();
    }

    Emissions m_Emissions;
  };
}

class QmitkSingleNodeSelectionWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkSingleNodeSelectionWidgetTestSuite);
  MITK_TEST(AutoSelectEnabledLast_SelectsMatch);
  MITK_TEST(AutoSelectEnabledFirst_KeepsMatch);
  MITK_TEST(AutoSelectEnabledFirst_WithTwoMatches_KeepsFirstMatch);
  MITK_TEST(RemoveSelectedNode_SelectsOtherMatch);
  MITK_TEST(RemoveOnlyMatch_SelectsNothing);
  MITK_TEST(OptionalClear_WithOneMatch_StaysEmpty);
  MITK_TEST(SwitchDataStorage_WithOldNodeAsOnlyMatch_SelectsItAgain);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Image1;
  mitk::DataNode::Pointer m_Image2;
  mitk::DataNode::Pointer m_Other;
  mitk::NodePredicateBase::Pointer m_IsImage;

  static mitk::DataNode::Pointer CreateNode(const std::string& name)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    return node;
  }

  /** Auto-select is enabled after the data storage is set. */
  void SetUpAutoSelectLast(TestWidget& widget) const
  {
    widget.SetNodePredicate(m_IsImage);
    widget.SetDataStorage(m_DataStorage);
    widget.SetAutoSelectNewNodes(true);
  }

  void SetUpAutoSelectFirst(TestWidget& widget) const
  {
    widget.SetNodePredicate(m_IsImage);
    widget.SetAutoSelectNewNodes(true);
    widget.SetDataStorage(m_DataStorage);
  }

  /** The data storage orders nodes by address, so which of two matches is
   *  picked first is arbitrary. Tests with two matches only rely on this. */
  mitk::DataNode::Pointer OtherImage(const mitk::DataNode* image) const
  {
    return image == m_Image1.GetPointer() ? m_Image2 : m_Image1;
  }

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Image1 = CreateNode("image1");
    m_Image2 = CreateNode("image2");
    m_Other = CreateNode("other");

    m_IsImage = mitk::NodePredicateFunction::New([](const mitk::DataNode* node) {
      return node->GetName().starts_with("image");
    }).GetPointer();

    m_DataStorage->Add(m_Image1);
    m_DataStorage->Add(m_Other);
  }

  void tearDown() override
  {
    m_IsImage = nullptr;
    m_Other = nullptr;
    m_Image2 = nullptr;
    m_Image1 = nullptr;
    m_DataStorage = nullptr;
  }

  void AutoSelectEnabledLast_SelectsMatch()
  {
    TestWidget widget;
    this->SetUpAutoSelectLast(widget);

    CPPUNIT_ASSERT_EQUAL(std::string("[{image1}]"), Describe(widget.m_Emissions));
    CPPUNIT_ASSERT(m_Image1 == widget.GetSelectedNode());
  }

  void AutoSelectEnabledFirst_KeepsMatch()
  {
    TestWidget widget;
    this->SetUpAutoSelectFirst(widget);

    CPPUNIT_ASSERT_EQUAL(std::string("[{image1}]"), Describe(widget.m_Emissions));
    CPPUNIT_ASSERT(m_Image1 == widget.GetSelectedNode());
  }

  void AutoSelectEnabledFirst_WithTwoMatches_KeepsFirstMatch()
  {
    m_DataStorage->Add(m_Image2);

    TestWidget widget;
    this->SetUpAutoSelectFirst(widget);

    const mitk::DataNode::Pointer firstMatch = m_DataStorage->GetNode(m_IsImage);
    CPPUNIT_ASSERT(firstMatch.IsNotNull());
    CPPUNIT_ASSERT_EQUAL(Describe({ { firstMatch } }), Describe(widget.m_Emissions));
    CPPUNIT_ASSERT(firstMatch == widget.GetSelectedNode());
  }

  void RemoveSelectedNode_SelectsOtherMatch()
  {
    m_DataStorage->Add(m_Image2);

    TestWidget widget;
    this->SetUpAutoSelectLast(widget);

    const auto removed = widget.GetSelectedNode();
    CPPUNIT_ASSERT(removed.IsNotNull());
    const auto remaining = this->OtherImage(removed);
    widget.m_Emissions.clear();

    m_DataStorage->Remove(removed);

    CPPUNIT_ASSERT_EQUAL(Describe({ { remaining } }), Describe(widget.m_Emissions));
    CPPUNIT_ASSERT(remaining == widget.GetSelectedNode());
  }

  void RemoveOnlyMatch_SelectsNothing()
  {
    TestWidget widget;
    this->SetUpAutoSelectLast(widget);
    widget.m_Emissions.clear();

    m_DataStorage->Remove(m_Image1);

    CPPUNIT_ASSERT_EQUAL(std::string("[{}]"), Describe(widget.m_Emissions));
    CPPUNIT_ASSERT(widget.GetSelectedNode().IsNull());
  }

  void OptionalClear_WithOneMatch_StaysEmpty()
  {
    TestWidget widget;
    widget.SetSelectionIsOptional(true);
    this->SetUpAutoSelectLast(widget);
    widget.m_Emissions.clear();

    widget.ClickClearButton();

    CPPUNIT_ASSERT_EQUAL(std::string("[{}]"), Describe(widget.m_Emissions));
    CPPUNIT_ASSERT(widget.GetSelectedNode().IsNull());
  }

  void SwitchDataStorage_WithOldNodeAsOnlyMatch_SelectsItAgain()
  {
    auto otherStorage = mitk::StandaloneDataStorage::New();
    otherStorage->Add(m_Image1);

    TestWidget widget;
    this->SetUpAutoSelectLast(widget);
    widget.m_Emissions.clear();

    widget.SetDataStorage(otherStorage);

    CPPUNIT_ASSERT_EQUAL(std::string("[{}, {image1}]"), Describe(widget.m_Emissions));
    CPPUNIT_ASSERT(m_Image1 == widget.GetSelectedNode());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkSingleNodeSelectionWidget)
