/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncDimension.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <nlohmann/json.hpp>

#include <QCoreApplication>

#include <string>

/**
 * Tests the interactive grid ops on QmitkMxNMultiWidget: AddGridColumn /
 * RemoveGridColumn / AddGridRow / RemoveGridRow and the tree-derived
 * ResolveGridShape they and the layout editor's buttons are gated on.
 *
 * The ops mutate the splitter tree in place, so existing windows keep their
 * ids, sync links, and positions; only the trailing edge changes. "Position"
 * throughout is the cell's index in ListWindowDescriptors() pre-order (the tree
 * walk); every grid here stays under ten cells and asserts via id-keyed
 * accessors, so the lexicographic GetNameFromIndex hazard never bites.
 */
class QmitkMxNGridOpsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNGridOpsTestSuite);
  MITK_TEST(AddGridColumn_PreservesLinks_AddsEmptyColumn);
  MITK_TEST(AddGridColumn_SerializeRoundTrips);
  MITK_TEST(RemoveGridColumn_DropsRightmost_GuardsAtOneColumn);
  MITK_TEST(RemoveGridColumn_ReclaimsEmptiedSelectionGroup);
  MITK_TEST(RemoveGridColumn_KeepsActive_WhenNotRemoved);
  MITK_TEST(RemoveGridColumn_RepointsActive_WhenRemoved);
  MITK_TEST(AddGridRow_AppendsBottomRow);
  MITK_TEST(RemoveGridRow_DropsBottom_NoOrphanSplitter_GuardsAtOneRow);
  MITK_TEST(GridOps_NoOpOnNonGridLayout);
  MITK_TEST(Maximize_ShowsOnlyTheTargetCell);
  MITK_TEST(Maximize_UnknownIdRestoresTheGrid);
  MITK_TEST(Maximize_IsInvisibleToSerialization);
  MITK_TEST(Maximize_LayoutChangeRestoresTheGrid);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    // Explicit node-level layer for the node-table-model comparator workaround
    // (see the note in QmitkMxNSyncGroupApiTest).
    m_Node = mitk::DataNode::New();
    m_Node->SetName("node");
    m_Node->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_Node);

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_Node = nullptr;
    m_DataStorage = nullptr;
  }

  /** The canonical id SetLayout assigns to the i-th created cell. */
  static QString CellId(int i)
  {
    return QStringLiteral("mxn__widget") + QString::number(i);
  }

  bool HasGroup(const std::string& id) const
  {
    for (const auto& info : m_Editor->GetSyncGroupInfos())
    {
      if (info.id == id)
      {
        return true;
      }
    }
    return false;
  }

  /** How many of the grid's cells are currently visible widgets. */
  int VisibleCellCount() const
  {
    int visible = 0;
    for (const auto& [id, cell] : m_Editor->GetRenderWindowWidgets())
    {
      if (cell->isVisible())
      {
        ++visible;
      }
    }
    return visible;
  }

  // ---------- Maximize (transient view state) ----------

  void Maximize_ShowsOnlyTheTargetCell()
  {
    m_Editor->SetLayout(2, 2);
    // The widgets must be realized for visibility to mean anything; without a
    // show the whole tree is hidden and the count below would be vacuously 0.
    m_Editor->show();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("All four cells start visible", 4, this->VisibleCellCount());

    m_Editor->SetMaximizedCell(CellId(2));

    CPPUNIT_ASSERT_EQUAL(CellId(2).toStdString(), m_Editor->GetMaximizedCell().toStdString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Only the maximized cell stays visible", 1,
                                 this->VisibleCellCount());
    CPPUNIT_ASSERT(m_Editor->GetRenderWindowWidgets().at(CellId(2))->isVisible());

    m_Editor->SetMaximizedCell(QString());

    CPPUNIT_ASSERT(m_Editor->GetMaximizedCell().isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Restoring brings every cell back", 4, this->VisibleCellCount());
  }

  void Maximize_UnknownIdRestoresTheGrid()
  {
    m_Editor->SetLayout(2, 2);
    m_Editor->show();
    m_Editor->SetMaximizedCell(CellId(1));

    m_Editor->SetMaximizedCell(QStringLiteral("mxn__nosuchwindow"));

    CPPUNIT_ASSERT_MESSAGE("An unknown id cannot leave cells hidden",
                           m_Editor->GetMaximizedCell().isEmpty());
    CPPUNIT_ASSERT_EQUAL(4, this->VisibleCellCount());
  }

  void Maximize_IsInvisibleToSerialization()
  {
    // Maximizing is a view state, not layout: it must not reach the document,
    // or reopening a saved layout would come back with cells missing.
    m_Editor->SetLayout(2, 2);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "sg");
    m_Editor->show();

    const auto plain = m_Editor->SerializeLayout();
    m_Editor->SetMaximizedCell(CellId(3));
    const auto maximized = m_Editor->SerializeLayout();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A maximized cell does not change the layout document",
                                 plain.dump(), maximized.dump());
  }

  void Maximize_LayoutChangeRestoresTheGrid()
  {
    m_Editor->SetLayout(2, 2);
    m_Editor->show();
    m_Editor->SetMaximizedCell(CellId(0));
    CPPUNIT_ASSERT_EQUAL(1, this->VisibleCellCount());

    // The cell set it was maximizing out of is gone; leaving siblings hidden
    // would strand the new grid. A rebuilt grid realizes its cells on the next
    // event-loop turn, so settle before counting.
    m_Editor->SetLayout(1, 3);
    QCoreApplication::processEvents();

    CPPUNIT_ASSERT_MESSAGE("A layout change drops the maximized state",
                           m_Editor->GetMaximizedCell().isEmpty());
    CPPUNIT_ASSERT_EQUAL(3, this->VisibleCellCount());
  }

  // ---------- AddGridColumn ----------

  void AddGridColumn_PreservesLinks_AddsEmptyColumn()
  {
    m_Editor->SetLayout(2, 2);
    // A distinct slice-sync group per original cell, so we verify each specific
    // link survives, not merely that some link exists.
    for (int i = 0; i < 4; ++i)
    {
      m_Editor->SetSyncLink(CellId(i), QmitkMxNSyncDimension::Slice, "sg" + std::to_string(i));
    }

    m_Editor->AddGridColumn();

    int rows = 0;
    int columns = 0;
    CPPUNIT_ASSERT(m_Editor->ResolveGridShape(rows, columns));
    CPPUNIT_ASSERT_EQUAL(2, rows);
    CPPUNIT_ASSERT_EQUAL(3, columns);
    CPPUNIT_ASSERT_EQUAL(3, m_Editor->GetColumnCount());
    CPPUNIT_ASSERT_EQUAL(2, m_Editor->GetRowCount());
    CPPUNIT_ASSERT_EQUAL(6u, m_Editor->GetNumberOfRenderWindowWidgets());

    for (int i = 0; i < 4; ++i)
    {
      const auto link = m_Editor->GetSyncLink(CellId(i), QmitkMxNSyncDimension::Slice);
      CPPUNIT_ASSERT(link.has_value());
      CPPUNIT_ASSERT_EQUAL(std::string("sg") + std::to_string(i), link.value().group);
    }

    // The two new trailing cells are empty: no links, default selection group.
    for (int i = 4; i <= 5; ++i)
    {
      CPPUNIT_ASSERT(nullptr != m_Editor->GetRenderWindowWidget(CellId(i)));
      CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(i), QmitkMxNSyncDimension::Slice).has_value());
      CPPUNIT_ASSERT_EQUAL(std::string("main"), m_Editor->GetCellSelectionGroup(CellId(i)));
    }
  }

  void AddGridColumn_SerializeRoundTrips()
  {
    m_Editor->SetLayout(2, 2);
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "shared");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "shared");
    m_Editor->AddGridColumn();

    const auto before = m_Editor->ListWindowDescriptors();

    nlohmann::json doc;
    CPPUNIT_ASSERT_NO_THROW(doc = m_Editor->SerializeLayout());
    CPPUNIT_ASSERT_NO_THROW(m_Editor->ApplyLayout(doc));

    const auto after = m_Editor->ListWindowDescriptors();
    CPPUNIT_ASSERT_EQUAL(before.size(), after.size());
    for (std::size_t i = 0; i < before.size(); ++i)
    {
      // Same cell at the same pre-order position, same selection group.
      CPPUNIT_ASSERT_EQUAL(before[i].id.toStdString(), after[i].id.toStdString());
      CPPUNIT_ASSERT_EQUAL(before[i].selectionGroup.toStdString(), after[i].selectionGroup.toStdString());
    }
    for (int i = 0; i <= 1; ++i)
    {
      const auto link = m_Editor->GetSyncLink(CellId(i), QmitkMxNSyncDimension::Slice);
      CPPUNIT_ASSERT(link.has_value());
      CPPUNIT_ASSERT_EQUAL(std::string("shared"), link.value().group);
    }

    int rows = 0;
    int columns = 0;
    CPPUNIT_ASSERT(m_Editor->ResolveGridShape(rows, columns));
    CPPUNIT_ASSERT_EQUAL(2, rows);
    CPPUNIT_ASSERT_EQUAL(3, columns);
  }

  // ---------- RemoveGridColumn ----------

  void RemoveGridColumn_DropsRightmost_GuardsAtOneColumn()
  {
    m_Editor->SetLayout(2, 3);  // row0: 0,1,2  row1: 3,4,5
    for (int i = 0; i < 6; ++i)
    {
      m_Editor->SetSyncLink(CellId(i), QmitkMxNSyncDimension::Slice, "keep");
    }

    m_Editor->RemoveGridColumn();

    int rows = 0;
    int columns = 0;
    CPPUNIT_ASSERT(m_Editor->ResolveGridShape(rows, columns));
    CPPUNIT_ASSERT_EQUAL(2, rows);
    CPPUNIT_ASSERT_EQUAL(2, columns);
    CPPUNIT_ASSERT_EQUAL(4u, m_Editor->GetNumberOfRenderWindowWidgets());
    CPPUNIT_ASSERT(nullptr == m_Editor->GetRenderWindowWidget(CellId(2)));  // rightmost row0
    CPPUNIT_ASSERT(nullptr == m_Editor->GetRenderWindowWidget(CellId(5)));  // rightmost row1
    for (const int i : {0, 1, 3, 4})
    {
      CPPUNIT_ASSERT(m_Editor->GetSyncLink(CellId(i), QmitkMxNSyncDimension::Slice).has_value());
    }

    // A survivor left with size 0 would serialize fine but be rejected on
    // reload; a round-trip proves the shrink leaves valid splitter sizes.
    nlohmann::json doc;
    CPPUNIT_ASSERT_NO_THROW(doc = m_Editor->SerializeLayout());
    CPPUNIT_ASSERT_NO_THROW(m_Editor->ApplyLayout(doc));
    CPPUNIT_ASSERT(m_Editor->ResolveGridShape(rows, columns));
    CPPUNIT_ASSERT_EQUAL(2, rows);
    CPPUNIT_ASSERT_EQUAL(2, columns);

    m_Editor->RemoveGridColumn();  // 2x2 -> 2x1
    CPPUNIT_ASSERT(m_Editor->ResolveGridShape(rows, columns));
    CPPUNIT_ASSERT_EQUAL(1, columns);
    const auto countAtOne = m_Editor->GetNumberOfRenderWindowWidgets();

    m_Editor->RemoveGridColumn();  // no-op: a grid must keep one column
    CPPUNIT_ASSERT_EQUAL(countAtOne, m_Editor->GetNumberOfRenderWindowWidgets());
  }

  void RemoveGridColumn_ReclaimsEmptiedSelectionGroup()
  {
    m_Editor->SetLayout(1, 2);  // widget0, widget1
    m_Editor->SetCellSelectionGroup(CellId(1), "solo");  // fresh connector, sole member
    CPPUNIT_ASSERT(HasGroup("solo"));

    m_Editor->RemoveGridColumn();  // removes widget1 (rightmost)

    CPPUNIT_ASSERT(nullptr == m_Editor->GetRenderWindowWidget(CellId(1)));
    CPPUNIT_ASSERT_MESSAGE("An emptied, method-allocated selection group must be reclaimed",
                           !HasGroup("solo"));
  }

  void RemoveGridColumn_KeepsActive_WhenNotRemoved()
  {
    m_Editor->SetLayout(2, 3);
    m_Editor->SetActiveRenderWindowWidget(m_Editor->GetRenderWindowWidget(CellId(0)));  // surviving column

    m_Editor->RemoveGridColumn();  // drops widget2, widget5

    const auto active = m_Editor->GetActiveRenderWindowWidget();
    CPPUNIT_ASSERT(nullptr != active);
    CPPUNIT_ASSERT_EQUAL(CellId(0).toStdString(), active->GetWidgetName().toStdString());
  }

  void RemoveGridColumn_RepointsActive_WhenRemoved()
  {
    m_Editor->SetLayout(2, 3);
    m_Editor->SetActiveRenderWindowWidget(m_Editor->GetRenderWindowWidget(CellId(2)));  // rightmost row0

    m_Editor->RemoveGridColumn();  // drops widget2 (active), widget5

    const auto active = m_Editor->GetActiveRenderWindowWidget();
    CPPUNIT_ASSERT(nullptr != active);
    const auto activeName = active->GetWidgetName();
    CPPUNIT_ASSERT_MESSAGE("Active must move off the removed cell to a survivor",
                           nullptr != m_Editor->GetRenderWindowWidget(activeName));
    CPPUNIT_ASSERT(activeName != CellId(2));
  }

  // ---------- AddGridRow / RemoveGridRow ----------

  void AddGridRow_AppendsBottomRow()
  {
    m_Editor->SetLayout(2, 2);
    m_Editor->AddGridRow();

    int rows = 0;
    int columns = 0;
    CPPUNIT_ASSERT(m_Editor->ResolveGridShape(rows, columns));
    CPPUNIT_ASSERT_EQUAL(3, rows);
    CPPUNIT_ASSERT_EQUAL(2, columns);
    CPPUNIT_ASSERT_EQUAL(3, m_Editor->GetRowCount());
    CPPUNIT_ASSERT_EQUAL(6u, m_Editor->GetNumberOfRenderWindowWidgets());

    // The new row is appended at the bottom: its cells are last in pre-order.
    const auto descriptors = m_Editor->ListWindowDescriptors();
    CPPUNIT_ASSERT_EQUAL(std::size_t{6}, descriptors.size());
    CPPUNIT_ASSERT_EQUAL(CellId(4).toStdString(), descriptors[4].id.toStdString());
    CPPUNIT_ASSERT_EQUAL(CellId(5).toStdString(), descriptors[5].id.toStdString());
  }

  void RemoveGridRow_DropsBottom_NoOrphanSplitter_GuardsAtOneRow()
  {
    m_Editor->SetLayout(2, 2);
    m_Editor->RemoveGridRow();  // drops widget2, widget3 and the emptied row-splitter

    int rows = 0;
    int columns = 0;
    // ResolveGridShape true is the orphan check: a leftover empty row-splitter
    // has zero cells, which would make it return false.
    CPPUNIT_ASSERT_MESSAGE("RemoveGridRow must leave no orphaned empty row-splitter",
                           m_Editor->ResolveGridShape(rows, columns));
    CPPUNIT_ASSERT_EQUAL(1, rows);
    CPPUNIT_ASSERT_EQUAL(2, columns);
    CPPUNIT_ASSERT_EQUAL(2u, m_Editor->GetNumberOfRenderWindowWidgets());

    // The tree walk tolerates no stray split node, and a survivor left with
    // size 0 would be rejected on reload; a round-trip proves both.
    nlohmann::json doc;
    CPPUNIT_ASSERT_NO_THROW(doc = m_Editor->SerializeLayout());
    CPPUNIT_ASSERT_NO_THROW(m_Editor->ApplyLayout(doc));
    CPPUNIT_ASSERT_EQUAL(2u, m_Editor->GetNumberOfRenderWindowWidgets());

    m_Editor->RemoveGridRow();  // no-op: a grid must keep one row
    CPPUNIT_ASSERT_EQUAL(2u, m_Editor->GetNumberOfRenderWindowWidgets());
  }

  // ---------- Guard ----------

  void GridOps_NoOpOnNonGridLayout()
  {
    // A horizontal root (as this doc builds) is not the vertical-root /
    // horizontal-rows grid shape, so ResolveGridShape is false and every op
    // no-ops.
    const auto doc = nlohmann::json::parse(R"json({
      "version": "3.0",
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__a", "view_direction": "axial", "links": { "selection": "main" } },
          { "type": "window", "id": "mxn__b", "view_direction": "axial", "links": { "selection": "main" } }
        ]
      }
    })json");
    m_Editor->ApplyLayout(doc);

    int rows = 0;
    int columns = 0;
    CPPUNIT_ASSERT(!m_Editor->ResolveGridShape(rows, columns));

    const auto before = m_Editor->GetNumberOfRenderWindowWidgets();
    m_Editor->AddGridColumn();
    m_Editor->AddGridRow();
    m_Editor->RemoveGridColumn();
    m_Editor->RemoveGridRow();
    CPPUNIT_ASSERT_EQUAL(before, m_Editor->GetNumberOfRenderWindowWidgets());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNGridOps)
