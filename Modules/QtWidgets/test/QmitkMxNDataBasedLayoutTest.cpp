/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkBaseRenderer.h>
#include <mitkImageGenerator.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

/**
 * Regression test for the data-based ("one row per image") layout.
 *
 * SetDataBasedLayout builds one synchronization group per selected image and
 * narrows each group's windows to show only that image. The regression: while
 * a later group's cells were being created, populating their sync-group
 * combobox emitted a spurious selection signal that transiently bound the new
 * cell to the lowest-numbered group and pushed a "select all" back into it,
 * re-showing the other image on the already-narrowed first row.
 *
 * The invariant asserted here is renderer-specific and independent of the
 * regression's mechanism: after SetDataBasedLayout, every window of a group
 * must resolve its own image visible and all other images hidden. Before the
 * fix this fails for group 1 (the other image resolves visible there).
 */
class QmitkMxNDataBasedLayoutTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNDataBasedLayoutTestSuite);
  MITK_TEST(SetDataBasedLayout_EachGroupShowsOnlyItsOwnImage);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node1;
  mitk::DataNode::Pointer m_Node2;

  static mitk::DataNode::Pointer MakeImageNode(const std::string& name, int layer)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    // SetDataBasedLayout reinitializes each window to the node's time geometry,
    // so the node must carry real image data.
    node->SetData(mitk::ImageGenerator::GenerateGradientImage<unsigned char>(8, 8, 8));
    // QmitkRenderWindowDataNodeTableModel comparator workaround (see
    // QmitkMxNSyncGroupApiTest for the long-form rationale).
    node->SetIntProperty("layer", layer);
    return node;
  }

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    m_Node1 = MakeImageNode("node1", 0);
    m_DataStorage->Add(m_Node1);

    m_Node2 = MakeImageNode("node2", 1);
    m_DataStorage->Add(m_Node2);
  }

  void tearDown() override
  {
    m_Node1 = nullptr;
    m_Node2 = nullptr;
    m_DataStorage = nullptr;
  }

  void SetDataBasedLayout_EachGroupShowsOnlyItsOwnImage()
  {
    auto editor = std::make_unique<QmitkMxNMultiWidget>();
    editor->SetDataStorage(m_DataStorage);
    editor->InitializeMultiWidget();

    // Row order defines group order: group 1 -> node1, group 2 -> node2.
    QList<mitk::DataNode::Pointer> nodes{ m_Node1, m_Node2 };
    editor->SetDataBasedLayout(nodes);

    const auto cells = editor->GetRenderWindowWidgets();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Two images, three view directions each -> six windows",
      std::size_t(6), cells.size());

    int inspected = 0;
    for (const auto& entry : cells)
    {
      const auto& cell = entry.second;
      auto* utility = cell->GetUtilityWidget();
      CPPUNIT_ASSERT(nullptr != utility);

      const int group = utility->GetSyncGroup();
      CPPUNIT_ASSERT_MESSAGE("Every cell must belong to group 1 or 2",
        group == 1 || group == 2);

      auto* renderer = mitk::BaseRenderer::GetInstance(cell->GetRenderWindow()->GetVtkRenderWindow());
      CPPUNIT_ASSERT(nullptr != renderer);

      const auto rowNode = (group == 1) ? m_Node1 : m_Node2;
      const auto otherNode = (group == 1) ? m_Node2 : m_Node1;

      CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "The group's own image must be visible in its windows",
        true, rowNode->IsVisible(renderer));
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "The other group's image must be hidden in this group's windows",
        false, otherNode->IsVisible(renderer));

      ++inspected;
    }
    CPPUNIT_ASSERT_EQUAL(6, inspected);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNDataBasedLayout)
