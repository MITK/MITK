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
#include <mitkImageGenerator.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <nlohmann/json.hpp>

#include <QCoreApplication>
#include <QSplitter>

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
  MITK_TEST(SetLayoutShrink_ReclaimsEmptiedSelectionGroup);
  MITK_TEST(RemoveGridColumn_KeepsActive_WhenNotRemoved);
  MITK_TEST(RemoveGridColumn_RepointsActive_WhenRemoved);
  MITK_TEST(AddGridRow_AppendsBottomRow);
  MITK_TEST(RemoveGridRow_DropsBottom_NoOrphanSplitter_GuardsAtOneRow);
  MITK_TEST(GridOps_NoOpOnNonGridLayout);
  MITK_TEST(Maximize_ShowsOnlyTheTargetCell);
  MITK_TEST(Maximize_UnknownIdRestoresTheGrid);
  MITK_TEST(Maximize_IsInvisibleToSerialization);
  MITK_TEST(Maximize_LayoutChangeRestoresTheGrid);
  MITK_TEST(Maximize_AddGridColumnRestoresTheGrid);
  MITK_TEST(Maximize_RemoveGridColumnRestoresTheGrid);
  MITK_TEST(Maximize_AddGridRowRestoresTheGrid);
  MITK_TEST(Maximize_RemoveGridRowRestoresTheGrid);
  MITK_TEST(Maximize_DataBasedLayoutRestoresTheGrid);
  MITK_TEST(Crosshair_NewCellsJoinTheEnabledCrosshair);
  MITK_TEST(Crosshair_NewCellsFollowVisibilityAndGap);
  MITK_TEST(NormalizedRects_MirrorTheGrid);
  MITK_TEST(NormalizedRects_FollowLoadedProportions);
  MITK_TEST(NormalizedRects_DescribeTheGridWhileMaximized);
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

  /** Lay out a grid in an editor with a real extent, settled. A splitter needs
   *  a size of its own before setSizes() means anything. */
  void SizedEditor(int rows, int columns) const
  {
    m_Editor->resize(1200, 800);
    m_Editor->show();
    m_Editor->SetLayout(rows, columns);
    QCoreApplication::processEvents();
  }

  /** Maximize a cell of a 2x2 grid, run a grid op, and check the op restored
   *  the grid. The maximize is taken over the cell set the op changes, and the
   *  splitter sizes it recorded would go stale (or dangle, for a removed row)
   *  if it survived. */
  void AssertGridOpRestoresTheGrid(void (QmitkMxNMultiWidget::*gridOp)(), int expectedCells) const
  {
    this->SizedEditor(2, 2);
    m_Editor->SetMaximizedCell(CellId(0));
    CPPUNIT_ASSERT_EQUAL(1, this->VisibleCellCount());

    (m_Editor.get()->*gridOp)();
    QCoreApplication::processEvents();

    CPPUNIT_ASSERT_MESSAGE("A grid op drops the maximized state",
                           m_Editor->GetMaximizedCell().isEmpty());
    CPPUNIT_ASSERT_EQUAL(expectedCells, this->VisibleCellCount());
  }

  /** The normalized rect the editor reports for a cell, or an invalid rect. */
  QRectF NormalizedRectOf(const QString& windowId) const
  {
    for (const auto& [id, rect] : m_Editor->GetNormalizedCellRects())
    {
      if (id == windowId)
      {
        return rect;
      }
    }
    return QRectF();
  }

  // ---------- Normalized cell rects (the layout map's geometry source) -------

  void NormalizedRects_MirrorTheGrid()
  {
    // These drive the arrange mode's Shift ranges. They come from the splitter
    // proportions rather than on-screen geometry precisely so they are right
    // without a layout pass - which is what this test relies on too.
    // A splitter has to have an extent before it can distribute one, so the
    // editor is sized and shown; on an unrealized widget the sizes come back
    // roughly even whatever was asked for.
    SizedEditor(2, 2);

    CPPUNIT_ASSERT_EQUAL(std::size_t(4), m_Editor->GetNormalizedCellRects().size());

    // Splitters deal in whole pixels, so an even grid can land a pixel off
    // centre. The tolerance is that rounding, not slack.
    const double pixelRounding = 0.02;

    const QRectF topLeft = NormalizedRectOf(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("Every cell is placed", topLeft.isValid());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, topLeft.x(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, topLeft.y(), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, topLeft.width(), pixelRounding);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, topLeft.height(), pixelRounding);

    const QRectF bottomRight = NormalizedRectOf(CellId(3));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, bottomRight.x(), pixelRounding);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, bottomRight.y(), pixelRounding);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The grid covers the full width",
                                         1.0, bottomRight.right(), pixelRounding);
  }

  void NormalizedRects_FollowLoadedProportions()
  {
    // The case that exposed this: a loaded layout is not a regular grid, so the
    // map cannot fall back on a row/column count and must read the document's
    // proportions. Round-tripping the editor's own document keeps the input
    // schema-valid while giving the two cells a 3:1 split.
    SizedEditor(1, 2);

    auto doc = m_Editor->SerializeLayout();
    auto& windows = doc["root"]["children"][0].contains("children")
      ? doc["root"]["children"][0]["children"]
      : doc["root"]["children"];
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The 1x2 document holds two windows",
                                 std::size_t(2), windows.size());
    // Pixels, not ratios: QSplitter::setSizes clamps anything below a child's
    // minimum width and splits the remainder evenly, so the values have to be
    // large enough to survive on a 1200 px editor.
    windows[0]["size"] = 900;
    windows[1]["size"] = 300;

    m_Editor->ApplyLayout(doc);
    QCoreApplication::processEvents();

    const double pixelRounding = 0.02;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The wide cell takes three quarters",
                                         0.75, NormalizedRectOf(CellId(0)).width(), pixelRounding);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The narrow cell starts at three quarters",
                                         0.75, NormalizedRectOf(CellId(1)).x(), pixelRounding);
  }

  void NormalizedRects_DescribeTheGridWhileMaximized()
  {
    // Maximizing hides the siblings, zeroing their splitter sizes. The map must
    // keep describing the grid the user will come back to, so the derivation
    // reads the proportions captured on the way in.
    m_Editor->SetLayout(2, 2);
    m_Editor->show();
    m_Editor->SetMaximizedCell(CellId(0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Every cell is still placed while maximized",
                                 std::size_t(4), m_Editor->GetNormalizedCellRects().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The maximized cell does not swallow the map",
                                         0.5, NormalizedRectOf(CellId(0)).width(), 1e-6);
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

  void Maximize_AddGridColumnRestoresTheGrid()
  {
    this->AssertGridOpRestoresTheGrid(&QmitkMxNMultiWidget::AddGridColumn, 6);
  }

  void Maximize_RemoveGridColumnRestoresTheGrid()
  {
    this->AssertGridOpRestoresTheGrid(&QmitkMxNMultiWidget::RemoveGridColumn, 2);
  }

  void Maximize_AddGridRowRestoresTheGrid()
  {
    this->AssertGridOpRestoresTheGrid(&QmitkMxNMultiWidget::AddGridRow, 6);
  }

  void Maximize_RemoveGridRowRestoresTheGrid()
  {
    this->AssertGridOpRestoresTheGrid(&QmitkMxNMultiWidget::RemoveGridRow, 2);
  }

  /** The crosshair node a cell's crosshair manager owns, or null while it is
   *  not in the data storage. */
  mitk::DataNode* CrosshairNodeOf(const QString& windowId) const
  {
    return m_DataStorage->GetNamedNode(windowId.toStdString() + "crosshairData");
  }

  void Crosshair_NewCellsJoinTheEnabledCrosshair()
  {
    m_Editor->SetLayout(2, 2);
    m_Editor->EnableCrosshair();

    m_Editor->AddGridColumn();
    m_Editor->AddGridRow();

    for (const auto& [id, cell] : m_Editor->GetRenderWindowWidgets())
    {
      CPPUNIT_ASSERT_MESSAGE("Every cell's crosshair reaches the data storage: " + id.toStdString(),
                             nullptr != this->CrosshairNodeOf(id));
    }
  }

  void Crosshair_NewCellsFollowVisibilityAndGap()
  {
    m_Editor->SetLayout(1, 2);
    m_Editor->EnableCrosshair();
    m_Editor->SetCrosshairVisibility(false);
    m_Editor->SetCrosshairGap(7);

    m_Editor->AddGridColumn();

    for (const auto& [id, cell] : m_Editor->GetRenderWindowWidgets())
    {
      CPPUNIT_ASSERT_MESSAGE("A new cell adopts the editor's crosshair visibility: " + id.toStdString(),
                             !cell->GetCrosshairVisibility());
      auto* node = this->CrosshairNodeOf(id);
      CPPUNIT_ASSERT(nullptr != node);
      int gap = 0;
      CPPUNIT_ASSERT(node->GetIntProperty("Crosshair.Gap Size", gap));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A new cell adopts the editor's crosshair gap: " + id.toStdString(), 7, gap);
    }
  }

  void Maximize_DataBasedLayoutRestoresTheGrid()
  {
    // SetDataBasedLayout reinitializes each window to the node's geometry, so
    // the node must carry real image data.
    auto image = mitk::DataNode::New();
    image->SetName("image");
    image->SetData(mitk::ImageGenerator::GenerateGradientImage<unsigned char>(8, 8, 8));
    image->SetIntProperty("layer", 1);
    m_DataStorage->Add(image);

    this->SizedEditor(2, 2);
    m_Editor->SetMaximizedCell(CellId(0));

    // The rebuild frees every splitter the maximize recorded sizes for.
    m_Editor->SetDataBasedLayout(QList<mitk::DataNode::Pointer>{ image });
    QCoreApplication::processEvents();

    CPPUNIT_ASSERT_MESSAGE("A data-based layout drops the maximized state",
                           m_Editor->GetMaximizedCell().isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One image, three view directions -> three visible windows",
                                 3, this->VisibleCellCount());
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

  void SetLayoutShrink_ReclaimsEmptiedSelectionGroup()
  {
    m_Editor->SetLayout(1, 2);  // widget0, widget1
    m_Editor->SetCellSelectionGroup(CellId(1), "solo");  // fresh connector, sole member
    CPPUNIT_ASSERT(HasGroup("solo"));

    m_Editor->SetLayout(1, 1);  // the shrink removes widget1

    CPPUNIT_ASSERT(nullptr == m_Editor->GetRenderWindowWidget(CellId(1)));
    CPPUNIT_ASSERT_MESSAGE("A shrink reclaims an emptied, method-allocated selection group",
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
