/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNLayoutEditorWidget.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QCoreApplication>

#include <algorithm>

/**
 * Drives the layout editor widget's mutation API against a real
 * QmitkMxNMultiWidget (no delegate, no mocks) and asserts the resulting
 * engine link state - the widget-level guarantee that survives the interim
 * sync popup's removal. The underlying engine behavior itself is covered by
 * QmitkMxNSyncGroupApiTest / QmitkMxNNavLinksTest.
 */
class QmitkMxNLayoutEditorWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNLayoutEditorWidgetTestSuite);

  MITK_TEST(CreateGroup_RegistersEngineGroup);
  MITK_TEST(Join_EmptyGroup_LinksNavigationBundleByDefault);
  MITK_TEST(Join_GroupWithDimensions_LinksThoseDimensions);
  MITK_TEST(AssignCells_JoinsEveryGivenCell);
  MITK_TEST(Leave_ClearsEveryDimension);
  MITK_TEST(ApplyDimension_TogglesForAllMembers);
  MITK_TEST(EditorChange_RefreshesCellBarcode);
  MITK_TEST(Selection_TogglesViaEditorAndRoundTrips);
  MITK_TEST(Selection_MoveIsSingleValued);
  MITK_TEST(Rebuild_PopulatesWithoutTouchingLinks);
  MITK_TEST(LayoutShrink_EmitsLayoutChanged);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;
  std::unique_ptr<QmitkMxNLayoutEditorWidget> m_Widget;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(1, 3);  // cells: mxn__widget0 .. mxn__widget2

    m_Widget = std::make_unique<QmitkMxNLayoutEditorWidget>();
    m_Widget->SetMultiWidget(m_Editor.get());
  }

  void tearDown() override
  {
    m_Widget.reset();
    m_Editor.reset();
    m_DataStorage = nullptr;
  }

  static QString CellId(std::size_t index)
  {
    return QStringLiteral("mxn__widget") + QString::number(index);
  }

  bool IsLinked(std::size_t cell, QmitkMxNSyncDimension dimension, const std::string& group) const
  {
    const auto link = m_Editor->GetSyncLink(CellId(cell), dimension);
    return link.has_value() && link->group == group;
  }

  void CreateGroup_RegistersEngineGroup()
  {
    const auto id = m_Widget->CreateGroup();
    CPPUNIT_ASSERT(!id.empty());

    const auto infos = m_Editor->GetSyncGroupInfos();
    const auto found = std::any_of(infos.begin(), infos.end(),
                                   [&id](const auto& info) { return info.id == id; });
    CPPUNIT_ASSERT_MESSAGE("A created group must appear in the group registry", found);
  }

  void Join_EmptyGroup_LinksNavigationBundleByDefault()
  {
    const auto id = m_Widget->CreateGroup();

    m_Widget->SetCellMembership(CellId(0), id, true);
    m_Widget->SetCellMembership(CellId(1), id, true);

    for (std::size_t cell : { std::size_t(0), std::size_t(1) })
    {
      for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                    QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair })
      {
        CPPUNIT_ASSERT_MESSAGE("Joining an empty group links the navigation bundle",
                               IsLinked(cell, dimension, id));
      }
      CPPUNIT_ASSERT_MESSAGE("Windowing is not part of the navigation bundle",
                             !IsLinked(cell, QmitkMxNSyncDimension::Windowing, id));
    }
    CPPUNIT_ASSERT_MESSAGE("A non-member stays unlinked",
                           !m_Editor->GetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice).has_value());
  }

  void Join_GroupWithDimensions_LinksThoseDimensions()
  {
    // Group already synchronizes windowing only (established via cell 0).
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "wl");

    m_Widget->SetCellMembership(CellId(1), "wl", true);

    CPPUNIT_ASSERT(IsLinked(1, QmitkMxNSyncDimension::Windowing, "wl"));
    CPPUNIT_ASSERT_MESSAGE("Joining follows the group's existing dimension set, not the bundle",
                           !IsLinked(1, QmitkMxNSyncDimension::Slice, "wl"));
  }

  void AssignCells_JoinsEveryGivenCell()
  {
    // The map's drag-and-drop / assign-selection path.
    const auto id = m_Widget->CreateGroup();

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(2) }, id);

    CPPUNIT_ASSERT(IsLinked(0, QmitkMxNSyncDimension::Slice, id));
    CPPUNIT_ASSERT(IsLinked(2, QmitkMxNSyncDimension::Slice, id));
    CPPUNIT_ASSERT_MESSAGE("The unassigned cell stays unlinked",
                           !m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice).has_value());
  }

  void Leave_ClearsEveryDimension()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "other");

    m_Widget->SetCellMembership(CellId(0), "nav", false);

    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice).has_value());
    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing).has_value());
    CPPUNIT_ASSERT_MESSAGE("Links to other groups survive leaving",
                           IsLinked(0, QmitkMxNSyncDimension::Zoom, "other"));
  }

  void ApplyDimension_TogglesForAllMembers()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    m_Widget->ApplyDimensionToGroup("nav", QmitkMxNSyncDimension::Windowing, true);
    CPPUNIT_ASSERT(IsLinked(0, QmitkMxNSyncDimension::Windowing, "nav"));
    CPPUNIT_ASSERT(IsLinked(1, QmitkMxNSyncDimension::Windowing, "nav"));
    CPPUNIT_ASSERT_MESSAGE("Non-members stay untouched",
                           !m_Editor->GetSyncLink(CellId(2), QmitkMxNSyncDimension::Windowing).has_value());

    m_Widget->ApplyDimensionToGroup("nav", QmitkMxNSyncDimension::Windowing, false);
    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing).has_value());
    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing).has_value());
    CPPUNIT_ASSERT_MESSAGE("Disabling a dimension keeps the membership dimension",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "nav"));
  }

  void EditorChange_RefreshesCellBarcode()
  {
    // Regression: an edit made in the layout editor must refresh the per-cell
    // utility-strip barcode, not just the editor's own view. The editor's
    // mutators route through RefreshSyncControls (SyncLinksChanged) for that;
    // before the fix they called only the editor-local rebuild, leaving the
    // barcode stale.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Widget->ApplyDimensionToGroup("nav", QmitkMxNSyncDimension::Windowing, true);

    const auto cell = m_Editor->GetRenderWindowWidget(CellId(0));
    CPPUNIT_ASSERT(nullptr != cell);
    auto* utility = cell->GetUtilityWidget();
    CPPUNIT_ASSERT(nullptr != utility);
    auto* barcode = utility->findChild<QmitkMxNSyncBarcodeWidget*>();
    CPPUNIT_ASSERT(nullptr != barcode);

    int windowingSlot = -1;
    for (std::size_t i = 0; i < QmitkMxNAllSyncDimensions.size(); ++i)
    {
      if (QmitkMxNAllSyncDimensions[i] == QmitkMxNSyncDimension::Windowing)
      {
        windowingSlot = static_cast<int>(i);
        break;
      }
    }
    CPPUNIT_ASSERT(windowingSlot >= 0);

    const auto axisSlots = barcode->Slots();
    CPPUNIT_ASSERT_MESSAGE(
      "An editor dimension toggle must refresh the cell's barcode without a manual refresh",
      axisSlots[windowingSlot].color.isValid());
  }

  void Selection_TogglesViaEditorAndRoundTrips()
  {
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);  // nav-links them

    m_Widget->ApplySelectionToGroup(id, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Enabling selection joins the members' selection group",
                                 id, m_Editor->GetCellSelectionGroup(CellId(0)));
    CPPUNIT_ASSERT_EQUAL(id, m_Editor->GetCellSelectionGroup(CellId(1)));
    CPPUNIT_ASSERT_MESSAGE("A non-member keeps the default selection group",
                           m_Editor->GetCellSelectionGroup(CellId(2)) != id);

    // The selection group survives a layout-document round-trip like any axis.
    const auto doc = m_Editor->SerializeLayout();
    m_Editor->ApplyLayout(doc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Selection group survives serialize/apply",
                                 id, m_Editor->GetCellSelectionGroup(CellId(0)));
    CPPUNIT_ASSERT_EQUAL(id, m_Editor->GetCellSelectionGroup(CellId(1)));

    m_Widget->ApplySelectionToGroup(id, false);
    CPPUNIT_ASSERT_MESSAGE("Disabling selection returns the cell to the default group",
                           m_Editor->GetCellSelectionGroup(CellId(0)) != id);
  }

  void Selection_MoveIsSingleValued()
  {
    // Cell 0 belongs to two groups on different dimensions, so it is a member of
    // both for the selection toggle.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "A");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "B");

    m_Widget->ApplySelectionToGroup("A", true);
    CPPUNIT_ASSERT_EQUAL(std::string("A"), m_Editor->GetCellSelectionGroup(CellId(0)));

    // Selection is single-valued per cell: enabling it on B moves the cell.
    m_Widget->ApplySelectionToGroup("B", true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The cell's single selection group moves A -> B",
                                 std::string("B"), m_Editor->GetCellSelectionGroup(CellId(0)));
  }

  void Rebuild_PopulatesWithoutTouchingLinks()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    // Let the coalesced rebuild run (registered via SetMultiWidget and the
    // engine's change signals).
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();

    CPPUNIT_ASSERT_MESSAGE("A rebuild must not mutate engine state",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "nav")
                             && IsLinked(1, QmitkMxNSyncDimension::Slice, "nav"));
  }

  void LayoutShrink_EmitsLayoutChanged()
  {
    // Removing cells through the layout controls must notify layout-tracking
    // furniture; a silent shrink leaves e.g. the layout editor rendering
    // dead cells.
    int layoutChanges = 0;
    const auto conn = QObject::connect(m_Editor.get(), &QmitkMxNMultiWidget::LayoutChanged,
                                       [&layoutChanges]() { ++layoutChanges; });

    m_Editor->SetLayout(1, 2);

    QObject::disconnect(conn);
    CPPUNIT_ASSERT_MESSAGE("Shrinking the layout must emit LayoutChanged", layoutChanges >= 1);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLayoutEditorWidget)
